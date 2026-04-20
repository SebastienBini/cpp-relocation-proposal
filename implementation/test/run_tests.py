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
from pathlib import Path
from typing import Optional


SCRIPT_DIR = Path(__file__).parent.resolve()
# Keep as clang++ (not resolved) so the C++ driver wrapper is used, which
# automatically links the C++ standard library.
DEFAULT_CLANGPP = SCRIPT_DIR / ".." / ".." / ".." / "build-make" / "bin" / "clang++"
CXX_FLAGS = ["-std=c++23", "-frelocation"]

# Matches a hex address like 0x7ffd3b3815c0
HEX_ADDR_RE = re.compile(r"0x[0-9a-fA-F]+")


def normalize_addresses(text: str) -> str:
    """Replace each distinct hex address with 0x1, 0x2, … in order of first appearance."""
    seen: dict[str, str] = {}
    def replacer(m: re.Match) -> str:
        addr = m.group(0)
        if addr not in seen:
            seen[addr] = f"0x{len(seen) + 1}"
        return seen[addr]
    return HEX_ADDR_RE.sub(replacer, text)


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


def run_test(cpp_path: Path, bless: bool = False, compiler: Optional[Path] = None) -> tuple:
    """
    Build and (if successful) run a single .cpp file.

    When *bless* is True, update the file's annotation with the actual
    (normalized) output instead of comparing.

    Returns (filename, passed, message).
    """
    name = cpp_path.name
    clangpp = compiler if compiler is not None else DEFAULT_CLANGPP

    # --- Build ---
    out_binary = cpp_path.with_suffix("")
    compile_result = subprocess.run(
        [str(clangpp)] + CXX_FLAGS + ["-o", str(out_binary), str(cpp_path)],
        capture_output=True,
        text=True,
    )

    if compile_result.returncode == 0:
        actual_outcome = "BUILD SUCCESS"
        # Run the binary to capture its output
        try:
            run_result = subprocess.run(
                [str(out_binary)],
                capture_output=True,
                text=True,
                timeout=30,
            )
            actual_output_raw = run_result.stdout + run_result.stderr
            program_exit_code = run_result.returncode
        finally:
            out_binary.unlink(missing_ok=True)
    else:
        actual_outcome = "BUILD FAILURE"
        actual_output_raw = compile_result.stderr + compile_result.stdout
        program_exit_code = None

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

    # --- Compare output (if any expected lines are present) ---
    if expected_lines:
        # Normalize the actual output as a whole (so address indices are
        # assigned in order of first appearance across all lines, matching
        # the behaviour of --bless which also normalizes the whole output).
        norm_expected = "\n".join(expected_lines).rstrip()
        norm_actual   = normalize_output(actual_output_raw, cpp_path).rstrip()

        if norm_actual != norm_expected:
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

    clangpp = Path(args.compiler) if args.compiler else DEFAULT_CLANGPP

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

    mode = "Blessing" if args.bless else "Running"
    print(f"{mode} {len(cpp_files)} test(s) with {clangpp.resolve()}\n")

    results: list[tuple] = []
    max_workers = min(len(cpp_files), os.cpu_count() or 4)
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as pool:
        futures = {pool.submit(run_test, p, args.bless, clangpp): p for p in cpp_files}
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

    print(f"\n{passed} passed, {failed} failed out of {len(results)} test(s).")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
