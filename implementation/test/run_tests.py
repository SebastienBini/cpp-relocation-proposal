#!/usr/bin/env python3
"""
Test runner for P2785 use-after-reloc tests.

For each *.cpp file in this directory, reads the expected outcome from the
////// BUILD SUCCESS|FAILURE tag and the commented expected output, builds
the file (in parallel), runs it if the build succeeded, and compares the
actual output to the expected output (normalizing memory addresses).
"""

import argparse
import concurrent.futures
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Optional


SCRIPT_DIR = Path(__file__).parent.resolve()
# Keep as clang++ (not resolved) so the C++ driver wrapper is used, which
# automatically links the C++ standard library.
DEFAULT_CLANGPP = SCRIPT_DIR / ".." / ".." / ".." / "build-make" / "bin" / "clang++"
DEFAULT_VALGRIND = Path("/opt/1A/toolchain/x86_64-v25.0.14/build-pack/25.0.14.0/bin/valgrind")
CXX_FLAGS = ["-std=c++2c", "-frelocation"]
DEFAULT_LLVM_ROOT = SCRIPT_DIR.parents[2]
DEFAULT_LIBCXX_INCLUDE = DEFAULT_LLVM_ROOT / "build-make" / "include" / "c++" / "v1"
DEFAULT_LIBCXX_LIB = DEFAULT_LLVM_ROOT / "build-make" / "lib"

# Matches a hex address like 0x7ffd3b3815c0
HEX_ADDR_RE = re.compile(r"0x[0-9a-fA-F]+")
# Matches a *normalized* address like 0x1, 0x12 (decimal digits only)
NORM_ADDR_RE = re.compile(r"0x[0-9]+")


def normalize_addresses(text: str) -> str:
    """Replace each distinct hex address with 0x1, 0x2, … in order of first appearance."""
    seen: dict[str, str] = {}
    def replacer(m: re.Match) -> str:
        addr = m.group(0)
        if addr not in seen:
            seen[addr] = f"0x{len(seen) + 1}"
        return seen[addr]
    return HEX_ADDR_RE.sub(replacer, text)


def libcxx_flags(libcxx_include: Path, libcxx_lib: Path) -> list[str]:
    """Return compiler/linker flags to force the local libc++ build."""
    return [
        "-nostdinc++",
        "-isystem",
        str(libcxx_include),
        "-stdlib=libc++",
        "-L",
        str(libcxx_lib),
        f"-Wl,-rpath,{libcxx_lib}",
    ]


def _has_decomposition_pack(libcxx_include: Path) -> bool:
    utility = libcxx_include / "utility"
    if not utility.exists():
        return False
    try:
        return "decomposition_pack" in utility.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return False


def _has_libcxx_libs(libcxx_lib: Path) -> tuple[bool, bool]:
    has_libcxx = any((libcxx_lib / name).exists() for name in ("libc++.so", "libc++.a"))
    has_libcxxabi = any((libcxx_lib / name).exists() for name in ("libc++abi.so", "libc++abi.a"))
    return has_libcxx, has_libcxxabi


def _discover_local_libcxx_roots() -> list[tuple[Path, Path]]:
    """Return (include_dir, lib_dir) candidates likely to be local libc++ builds."""
    roots = [DEFAULT_LLVM_ROOT / "build-make", DEFAULT_LLVM_ROOT / "build"]
    include_candidates: list[Path] = []

    # Fast paths first.
    for root in roots:
        include_candidates.append(root / "include" / "c++" / "v1")

    # Then look for nested build layouts used by runtimes/libc++ builds.
    for root in roots:
        if root.exists():
            for utility in root.glob("**/include/c++/v1/utility"):
                include_candidates.append(utility.parent)

    # Deduplicate while preserving discovery order.
    seen: set[Path] = set()
    unique_includes: list[Path] = []
    for inc in include_candidates:
        resolved = inc.resolve()
        if resolved not in seen:
            seen.add(resolved)
            unique_includes.append(resolved)

    out: list[tuple[Path, Path]] = []
    for inc in unique_includes:
        # include/c++/v1 => prefix is 3 levels up
        prefix = inc.parents[2] if len(inc.parents) >= 3 else inc
        out.append((inc, prefix / "lib"))
    return out


def resolve_local_libcxx_paths(
    include_override: Optional[str],
    lib_override: Optional[str],
) -> tuple[Optional[Path], Optional[Path], Optional[str]]:
    """
    Resolve local libc++ include/lib paths.

    Returns (include, lib, error). If include/lib are not None, they are usable.
    """
    if include_override or lib_override:
        libcxx_include = Path(include_override) if include_override else DEFAULT_LIBCXX_INCLUDE
        libcxx_lib = Path(lib_override) if lib_override else DEFAULT_LIBCXX_LIB
        if not _has_decomposition_pack(libcxx_include):
            return None, None, (
                "Selected libc++ headers do not provide std::decomposition_pack in <utility>: "
                f"{libcxx_include / 'utility'}"
            )
        has_libcxx, has_libcxxabi = _has_libcxx_libs(libcxx_lib)
        if not has_libcxx or not has_libcxxabi:
            return None, None, (
                "Selected libc++ lib dir is missing libc++ or libc++abi: "
                f"{libcxx_lib}"
            )
        return libcxx_include, libcxx_lib, None

    # Try default, then auto-discovery.
    for libcxx_include, libcxx_lib in [(DEFAULT_LIBCXX_INCLUDE, DEFAULT_LIBCXX_LIB), *_discover_local_libcxx_roots()]:
        if not _has_decomposition_pack(libcxx_include):
            continue
        has_libcxx, has_libcxxabi = _has_libcxx_libs(libcxx_lib)
        if has_libcxx and has_libcxxabi:
            return libcxx_include, libcxx_lib, None

    return None, None, (
        "Could not find a local libc++ build with std::decomposition_pack in <utility> and "
        "matching libc++/libc++abi libraries."
    )


def normalize_output(text: str, cpp_path: Path) -> str:
    """
    Normalize compiler/program output for comparison:
    - Replace the absolute path of the source file with just the filename.
    - Replace each distinct hex address with 0x1, 0x2, … in order of first appearance.
    """
    # Replace absolute path references to the source file with just the filename
    text = text.replace(str(cpp_path), cpp_path.name)
    return normalize_addresses(text)


def parse_expected(cpp_path: Path) -> Optional[tuple[str, list[str]]]:
    """
    Parse the expected result embedded in the .cpp file.

    Returns (outcome, lines) where outcome is 'BUILD SUCCESS' or 'BUILD FAILURE'
    and lines is the list of expected output lines (stripped of the leading '// ').
    Returns None if no tag is found.
    """
    outcome = None
    output_lines: list[str] = []
    in_output = False

    for raw in cpp_path.read_text(encoding="utf-8").splitlines():
        line = raw.rstrip()
        if line.startswith("////// BUILD SUCCESS"):
            outcome = "BUILD SUCCESS"
            in_output = True
        elif line.startswith("////// BUILD FAILURE"):
            outcome = "BUILD FAILURE"
            in_output = True
        elif in_output:
            if line.startswith("// "):
                output_lines.append(line[3:])
            elif line == "//":
                output_lines.append("")
            elif line == "":
                # blank lines after the tag section are fine, keep collecting
                pass
            else:
                # non-comment line after the tag – stop
                in_output = False

    return (outcome, output_lines) if outcome is not None else None


def write_annotation(cpp_path: Path, outcome: str, output_raw: str) -> None:
    """
    Rewrite the ////// BUILD … section at the end of *cpp_path* with *outcome*
    and the normalized *output_raw* lines as '// …' comments.
    """
    text = cpp_path.read_text(encoding="utf-8")
    # Drop everything from the first ////// BUILD line to end-of-file
    tag_re = re.compile(r"\n////// BUILD.*", re.DOTALL)
    text = tag_re.sub("", text).rstrip()

    norm_lines = normalize_output(output_raw, cpp_path).splitlines()
    # Remove trailing empty lines from output
    while norm_lines and not norm_lines[-1].strip():
        norm_lines.pop()

    annotation = f"\n\n////// {outcome}\n"
    if norm_lines:
        annotation += "\n".join(f"// {l}" if l else "//" for l in norm_lines) + "\n"

    cpp_path.write_text(text + annotation, encoding="utf-8")


def build_only(cpp_path: Path, cxx_flags: list[str], compiler: Optional[Path] = None) -> tuple:
    """
    Build *cpp_path* and keep the resulting binary. Does not run, bless, or compare.

    Returns (filename, passed, message).
    """
    name = cpp_path.name
    clangpp = compiler if compiler is not None else DEFAULT_CLANGPP
    out_binary = cpp_path.with_suffix("")
    compile_result = subprocess.run(
        [str(clangpp)] + cxx_flags + ["-o", str(out_binary), str(cpp_path)],
        capture_output=True,
        text=True,
    )
    if compile_result.returncode == 0:
        return name, True, f"BUILT -> {out_binary}"
    else:
        output = (compile_result.stderr + compile_result.stdout).rstrip()
        return (
            name,
            False,
            f"BUILD FAILED:\n" + "\n".join(f"    {l}" for l in output.splitlines()),
        )


def run_test(cpp_path: Path, cxx_flags: list[str], bless: bool = False, compiler: Optional[Path] = None,
             valgrind: Optional[Path] = None) -> tuple:
    """
    Build and (if successful) run a single .cpp file.

    When *bless* is True, update the file's annotation with the actual
    (normalized) output instead of comparing.
    When *valgrind* is set, the program is run under valgrind memcheck and
    the valgrind log is checked for memory errors; the program output is
    still compared against the expected output.

    Returns (filename, passed, message).
    """
    name = cpp_path.name
    clangpp = compiler if compiler is not None else DEFAULT_CLANGPP

    # --- Build ---
    out_binary = cpp_path.with_suffix("")
    compile_result = subprocess.run(
        [str(clangpp)] + cxx_flags + ["-o", str(out_binary), str(cpp_path)],
        capture_output=True,
        text=True,
    )

    if compile_result.returncode == 0:
        actual_outcome = "BUILD SUCCESS"
        # Run the binary (optionally under valgrind) to capture its output
        try:
            if valgrind is not None:
                vg_log_fd, vg_log_path_str = tempfile.mkstemp(suffix=".log", prefix="vg_")
                os.close(vg_log_fd)
                vg_log_path = Path(vg_log_path_str)
                try:
                    run_result = subprocess.run(
                        [
                            str(valgrind),
                            "--tool=memcheck",
                            "--leak-check=full",
                            f"--log-file={vg_log_path}",
                            str(out_binary),
                        ],
                        capture_output=True,
                        timeout=120,
                    )
                    actual_output_raw = (
                        run_result.stdout.decode("utf-8", errors="replace")
                        + run_result.stderr.decode("utf-8", errors="replace")
                    )
                    program_exit_code = run_result.returncode
                    vg_log = vg_log_path.read_text(encoding="utf-8", errors="replace") if vg_log_path.exists() else ""
                finally:
                    vg_log_path.unlink(missing_ok=True)
            else:
                run_result = subprocess.run(
                    [str(out_binary)],
                    capture_output=True,
                    timeout=30,
                )
                actual_output_raw = (
                    run_result.stdout.decode("utf-8", errors="replace")
                    + run_result.stderr.decode("utf-8", errors="replace")
                )
                program_exit_code = run_result.returncode
                vg_log = None
        finally:
            out_binary.unlink(missing_ok=True)
    else:
        actual_outcome = "BUILD FAILURE"
        actual_output_raw = compile_result.stderr + compile_result.stdout
        program_exit_code = None
        vg_log = None

    # --- Check for unexpected non-zero exit codes ---
    # The compiler should always exit cleanly (rc=0 for success, rc=1 for
    # diagnosed errors). Any other code (e.g. crash / signal) is reported.
    compiler_rc = compile_result.returncode
    if compiler_rc not in (0, 1):
        return (
            name,
            False,
            f"COMPILER CRASHED (exit code {compiler_rc}):\n"
            + "\n".join(f"    {l}" for l in actual_output_raw.splitlines()),
        )

    # The program must exit with code 0.
    if program_exit_code is not None and program_exit_code != 0:
        return (
            name,
            False,
            f"PROGRAM CRASHED OR FAILED (exit code {program_exit_code}):\n"
            + "\n".join(f"    {l}" for l in actual_output_raw.splitlines()),
        )

    # --- Valgrind check ---
    if vg_log is not None:
        # valgrind always writes "ERROR SUMMARY: N errors" at the end of the log.
        # With --leak-check=full, definitely/indirectly lost bytes are counted.
        m = re.search(r"ERROR SUMMARY:\s*(\d+)\s*error", vg_log)
        if m is None or int(m.group(1)) != 0:
            return (
                name,
                False,
                f"VALGRIND ERRORS DETECTED:\n"
                + "\n".join(f"    {l}" for l in vg_log.splitlines()),
            )

    # --- Bless mode: update the file and return ---
    if bless:
        write_annotation(cpp_path, actual_outcome, actual_output_raw)
        return name, True, f"BLESSED ({actual_outcome})"

    # --- Compare mode ---
    expected = parse_expected(cpp_path)
    if expected is None:
        return name, False, "SKIP: no ////// BUILD SUCCESS|FAILURE tag found"

    expected_outcome, expected_lines = expected

    # --- Compare outcome ---
    if actual_outcome != expected_outcome:
        return (
            name,
            False,
            f"OUTCOME MISMATCH: expected {expected_outcome}, got {actual_outcome}\n"
            f"  Actual output:\n"
            + "\n".join(f"    {l}" for l in actual_output_raw.splitlines()),
        )

    # --- Compare output ---
    # Normalize the actual output as a whole (so address indices are
    # assigned in order of first appearance across all lines, matching
    # the behaviour of --bless which also normalizes the whole output).
    norm_expected = "\n".join(expected_lines).rstrip()
    norm_actual   = normalize_output(actual_output_raw, cpp_path).rstrip()

    # Under valgrind the allocator reuses memory differently, so the
    # normalized address indices (0x1, 0x2, …) may differ while the
    # rest of the output is identical.  Mask all normalized addresses
    # before comparing so that only structural differences are flagged.
    if valgrind is not None:
        cmp_expected = NORM_ADDR_RE.sub("0x_", norm_expected)
        cmp_actual   = NORM_ADDR_RE.sub("0x_", norm_actual)
    else:
        cmp_expected = norm_expected
        cmp_actual   = norm_actual

    if cmp_actual != cmp_expected:
        return (
            name,
            False,
            f"OUTPUT MISMATCH:\n"
            f"  Expected:\n"
            + "\n".join(f"    {l}" for l in norm_expected.splitlines())
            + "\n  Actual:\n"
            + "\n".join(f"    {l}" for l in norm_actual.splitlines()),
        )

    return name, True, f"OK ({actual_outcome})"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build and run P2785 relocation tests."
    )
    parser.add_argument(
        "--compiler",
        metavar="PATH",
        default=None,
        help=(
            f"Path to the clang++ binary to use for building "
            f"(default: {DEFAULT_CLANGPP})."
        ),
    )
    parser.add_argument(
        "--stdlib",
        choices=["local-libc++", "system"],
        default="local-libc++",
        help=(
            "C++ standard library to use. 'local-libc++' forces the local "
            "libc++ build; 'system' uses the compiler default STL. "
            "(default: local-libc++)."
        ),
    )
    parser.add_argument(
        "--libcxx-include",
        metavar="PATH",
        default=None,
        help=(
            "Path to libc++ headers directory (the one containing <utility>, "
            "typically .../include/c++/v1). "
            f"(default: {DEFAULT_LIBCXX_INCLUDE})."
        ),
    )
    parser.add_argument(
        "--libcxx-lib",
        metavar="PATH",
        default=None,
        help=(
            "Path to directory containing libc++ libraries (libc++/libc++abi). "
            f"(default: {DEFAULT_LIBCXX_LIB})."
        ),
    )
    parser.add_argument(
        "--valgrind",
        action="store_true",
        help=(
            "Run successfully-built binaries under valgrind memcheck. "
            "Program output is still compared against the expected output. "
            "Memory errors (leaks, corruption, …) cause the test to fail."
        ),
    )
    parser.add_argument(
        "--valgrind-path",
        metavar="PATH",
        default=None,
        help=(
            f"Path to the valgrind binary (default: {DEFAULT_VALGRIND}). "
            "Implies --valgrind."
        ),
    )
    parser.add_argument(
        "--build",
        action="store_true",
        help=(
            "Build the listed source file(s) and keep the resulting binary. "
            "Does not run the program, update annotations, or compare output. "
            "Requires explicit FILE argument(s)."
        ),
    )
    parser.add_argument(
        "--bless",
        action="store_true",
        help=(
            "Instead of comparing against the expected output, rebuild every "
            "test and overwrite its ////// BUILD SUCCESS|FAILURE annotation "
            "with the actual (normalized) output."
        ),
    )
    parser.add_argument(
        "files",
        nargs="*",
        metavar="FILE",
        help="Specific .cpp file(s) to process (default: all *.cpp in this directory).",
    )
    args = parser.parse_args()

    if args.build and not args.files:
        parser.error("--build requires at least one FILE argument")

    if args.build and args.bless:
        print("error: --build and --bless are mutually exclusive.")
        return 1

    if args.build and args.valgrind:
        print("error: --build and --valgrind are mutually exclusive.")
        return 1

    clangpp = Path(args.compiler) if args.compiler else DEFAULT_CLANGPP
    valgrind: Optional[Path] = None
    if args.valgrind_path:
        valgrind = Path(args.valgrind_path)
    elif args.valgrind:
        valgrind = DEFAULT_VALGRIND
    if valgrind is not None and not valgrind.exists():
        print(f"Valgrind not found: {valgrind}")
        return 1

    if args.files:
        cpp_files = sorted(Path(f).resolve() for f in args.files)
    else:
        cpp_files = sorted(SCRIPT_DIR.glob("*.cpp"))

    if not cpp_files:
        print("No .cpp files found.")
        return 1

    if not clangpp.exists():
        print(f"Compiler not found: {clangpp.resolve()}")
        return 1

    cxx_flags = list(CXX_FLAGS)
    if args.stdlib == "local-libc++":
        libcxx_include, libcxx_lib, err = resolve_local_libcxx_paths(args.libcxx_include, args.libcxx_lib)
        if err is not None:
            print(err)
            print("Provide --libcxx-include/--libcxx-lib, or build libc++ + libc++abi first.")
            return 1

        cxx_flags += libcxx_flags(libcxx_include, libcxx_lib)

    mode = "Building" if args.build else "Blessing" if args.bless else "Running"
    vg_suffix = f" (valgrind {valgrind})" if valgrind else ""
    stdlib_desc = args.stdlib
    print(f"{mode} {len(cpp_files)} test(s) with {clangpp.resolve()} [{stdlib_desc}]{vg_suffix}\n")

    results: list[tuple] = []
    max_workers = min(len(cpp_files), os.cpu_count() or 4)
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as pool:
        if args.build:
            futures = {pool.submit(build_only, p, cxx_flags, clangpp): p for p in cpp_files}
        else:
            futures = {pool.submit(run_test, p, cxx_flags, args.bless, clangpp, valgrind): p for p in cpp_files}
        for fut in concurrent.futures.as_completed(futures):
            results.append(fut.result())

    # Sort by filename for deterministic output
    results.sort(key=lambda r: r[0])

    passed = 0
    failed = 0
    for name, ok, msg in results:
        status = "PASS" if ok else "FAIL"
        print(f"  [{status}] {name}: {msg}")
        if ok:
            passed += 1
        else:
            failed += 1

    if args.bless:
        print(f"\n{passed} file(s) blessed.")
        return 0

    if args.build:
        print(f"\n{passed} built, {failed} failed out of {len(results)} file(s).")
        return 0 if failed == 0 else 1

    print(f"\n{passed} passed, {failed} failed out of {len(results)} test(s).")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
