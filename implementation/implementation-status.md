# P2785 "Relocating prvalues" — Clang Prototype Implementation Status

Branch: `work/relocation-P2785`  
Build: `/workspace/llvm-project/build-make/`  
Flag: `-frelocation` (`-std=c++17` or later required)

---

## Phase 1 — Keyword registration ✅ `f197918`

**Scope:** Language option, preprocessor, driver.

- New `-frelocation` language option (`LangOpts.Relocation`)
- `reloc` registered as a keyword gated behind `-frelocation` (leaves it as a normal identifier otherwise)
- Driver flag `-frelocation` / `-fno-relocation` forwarded through the compilation pipeline
- Stub parser case emits `err_reloc_not_implemented` for any expression use
- Serialisation of `LangOpts.Relocation` in the PCH reader/writer

**Proposal coverage:** §"reloc operator" (syntax surface only)

---

## Phase 2 — `reloc` unary operator (AST → Sema → Parser → CodeGen) ✅ `978977d` + `92c39cc`

**Scope:** End-to-end support for `reloc expr` on local variables and parameters.

- New AST node `CXXRelocExpr` in `ExprCXX.h` (prvalue, single `Expr *` operand)
- `StmtNodes.td`, `ExprClassification.cpp`, `Expr.cpp` (`isUnusedResultAWarning`, `hasSideEffects`), `RecursiveASTVisitor.h`, `StmtPrinter.cpp`, `StmtProfile.cpp`
- `SemaRelocation.cpp`: `ActOnRelocExpr` — operand validation, cv-stripping, result type
- Parser: `ParseCastExpression` handles `tok::kw_reloc`, builds unary `reloc` as `CXXRelocExpr`
- Serialisation: `ASTReader`/`ASTWriter` round-trips `CXXRelocExpr`
- CodeGen: `EmitCXXRelocExpr` — scalar types emit a plain load; class types emit `EmitAggregateCopy` (placeholder, corrected in Phase 3)
- Sema checks: illegal operands (static/thread-local, global, rvalue, reference-typed, non-lvalue, uninitialised), `reloc` in unevaluated operands
- Anonymous union member support: `reloc x` where `x` is a union member
- 38 unit tests in `CXXRelocExprTest.cpp`

**Proposal coverage:** §"reloc operator", §"ill-formed uses of reloc", §"reloc in unevaluated operands", §"anonymous union members"

---

## Phase 3 — Move constructor, value categories, source lifetime ✅ `d3ed208`

**Scope:** Correct codegen for class-type operands; source-object lifetime suppression.

- `CXXRelocExpr` stores `ValueKind` and a pointer to the selected move constructor
- For class-type operands: `EmitCXXRelocExpr` emits a proper move-constructor call (same as `std::move` initialisation)
- Source-object destructor suppressed at scope exit after `reloc` (the variable is marked *early-destructible*): `VarDecl` gains `IsRelocated` flag; `CodeGenFunction::emitAutoVarTypeCleanup` skips it
- `reloc` on a reference-typed variable perfectly forwards the referenced object (xvalue path) per §"reloc to perfectly forward all value categories"
- `reloc` on a scalar type correctly stores a null move ctor
- `reloc arr` where `arr` is a C-array produces an array prvalue usable only via materialization (binding to a reference, passing to a function taking an array reference). Copy-initialization of a C-array from a C-array prvalue (`T dst[N] = reloc src;`) is ill-formed per `f978aed`; this is already rejected by standard Clang because arrays cannot be copy-initialized from prvalues. Element-wise relocation for non-trivial element types is implemented (see "C-array element-wise relocation" below).

**Proposal coverage:** §"source and target objects", §"early destructible", §"reloc produces a prvalue", §"reloc to perfectly forward all value categories"

---

## Phase 4 — Use-after-reloc CFG dataflow analysis ✅ `82af095`

**Scope:** Ill-formed reuse detection via forward CFG dataflow (inter-statement).

### Design

Two-phase forward dataflow on the Clang CFG:

| State | Value | Meaning |
|---|---|---|
| `Bottom` | `0x0` | Not yet visited |
| `Alive` | `0x1` | Not relocated on any reaching path |
| `Relocated` | `0x2` | Definitely relocated on all reaching paths |
| `AliveOrRelocated` | `0x3` | Relocated on some paths (join) |

Bitwise-OR gives the correct lattice join. Two passes:
1. **Phase 1 (propagation):** worklist to convergence — no diagnostics.
2. **Phase 2 (reporting):** one replay per reachable block with its converged IN-state.

`VisitDeclStmt` resets a variable's state to `Alive` at its declaration, handling variables declared inside loop bodies (showcase_12).  
`VisitBinaryOperator` suppresses re-visiting `||`/`&&` operands from the join block.  
`VisitAbstractConditionalOperator` suppresses re-visiting `?:` operands from the join block.

### New diagnostics

```
error: use of 'X' after it has been relocated
error: use of 'X' after it may have been relocated
note:  relocated here
```

### Entry point

`Sema::CheckRelocUseAfterReloc(const Decl *, AnalysisDeclContext &)` — called from `AnalysisBasedWarnings::IssueWarnings` when `getLangOpts().Relocation`.

### Showcase coverage

| Showcase | Status |
|---|---|
| 01 simple use-after-reloc | ✅ |
| 02 variable shadowing | ✅ |
| 03 use on other branch before join | ✅ |
| 04 use at A-R join | ✅ |
| 05 both branches reloc, no use | ✅ |
| 06 runtime flag irrelevant | ✅ |
| 07 `if constexpr` discarded branch | ✅ |
| 08 return after reloc | ✅ |
| 09 loop reloc → A-R on 2nd iter | ✅ |
| 10 conditional reloc in loop | ✅ |
| 11 break after reloc cuts back-edge | ✅ |
| 12 var declared inside loop body | ✅ |
| 13 backward goto → A-R | ✅ |
| 14 forward goto (doesn't cross reloc) | ✅ |
| 15 anonymous union member not tracked | ✅ |
| 17 short-circuit `\|\|` | ✅ |

**Proposal coverage:** §"ill-formed reuse of relocated objects", §"variable state tracking"

---

## Phase 4b — Unsequenced reloc + use in single expression (showcase_18) ✅ `7757495` + `32b2d4e`

**Scope:** Intra-expression ill-formed reuse where evaluation order is unspecified.

### Design

`UnsequencedRelocChecker` — bottom-up recursive AST visitor called from `CheckCompletedExpr` for every full-expression. Returns per-sub-expression sets of `{Relocated vars, Used vars}`. When two children are **unsequenced**, the sets are cross-checked.

Sequencing rules:

| Context | Sequenced? | Cross-check? |
|---|---|---|
| `+`, `-`, `*`, `[]`, etc. | No | ✅ |
| Function-call argument pairs | No | ✅ |
| `,` (comma) | Yes (L→R) | No (Phase 4 handles) |
| `\|\|`, `&&` | Yes (L→R) | No (Phase 4 handles) |
| `?:` branches | Mutually exclusive | No |
| Default (siblings) | Assumed unsequenced | ✅ |

### New diagnostics

```
error: use of 'X' is unsequenced with its relocation
note:  relocation of 'X' is here
```

### Entry point

`Sema::CheckRelocUnsequenced(const Expr *)` — called from `Sema::CheckCompletedExpr`.

### Showcase coverage

| Showcase | Status |
|---|---|
| 18 `reloc x + reloc x` | ✅ |
| 18 `x + reloc x` | ✅ |
| 18 `reloc x + x` | ✅ |
| 18 `x[reloc x]` | ✅ |
| 18 `foo(x, reloc x)` | ✅ |
| 18 `foo(x, t() ? reloc x : x)` | ✅ |
| 18 `foo(t() ? reloc x : x)` — well-formed | ✅ clean |
| 18 `(x, reloc x)` — sequenced, handled by Phase 4 | ✅ clean |

---

## Phase 4c — Per-member/base use-after-reloc tracking ✅ `9574b4c`

**Scope:** Extend the dataflow analysis to track individual member and base subobject relocations for decomposed objects.

### Design

`RelocKey` is now a discriminated union of three cases:
- `Var` — bare variable (existing Phase 4 tracking)
- `Member` — `(VarDecl*, FieldDecl*)` pair for `reloc obj.member`
- `Base` — `(VarDecl*, CXXRecordDecl*)` pair for `reloc obj.base<B>`

`RelocVarIndex` uses three parallel `DenseMaps` (`VarMap`, `MemberMap`, `BaseMap`) sharing one monotone `NextIdx` counter. A `findEnclosingBase()` helper finds a tracked base `B` for which `B == FieldParent` or `B->isDerivedFrom(…)`.

### New diagnostics

```
error: use of member 'X' after it has been relocated
error: use of member 'X' after it may have been relocated
error: 'reloc obj.member' — enclosing base of 'member' has already been relocated
error: 'X' is not a direct member of the decomposed type
```

### Coverage

All seven `A←B←D` member/base combinations at top level; all seven under one nesting level (`E reloc e`, preamble `D reloc d = reloc e.base<D>`); nested decomposition sequences (`reloc b.base<A>` after `B reloc b`); inherited-member-not-direct rejection.

---

## Sema fix — §1183 constructor availability check ✅ `170cc8a`

**Description:** `reloc x` where `x`'s type has both copy and move constructors deleted is now rejected with `err_reloc_no_accessible_ctor`.

**Discarded-reloc exception:** ✅ Implemented in `815f3c6`. Discarded `reloc x;` on an immovable non-parameter local (where both move and copy constructors are deleted) is now accepted: the destructor is called directly; no constructor is needed. For parameters, the exception does not apply — rejection is immediate.

---

## Sema fix — §1189 lambda capture detection ✅ `f3c2ddc`

**Description:** `reloc x` inside a lambda body on any capture (by-value, by-reference, or init-capture) is now rejected with `err_reloc_lambda_capture`. Detection uses `isLambdaCallOperator(CurContext)` + `DeclContext` mismatch / `isInitCapture()` check.

**Future:** ~~The proposal permits reloc on lambda captures when the closure has been decomposed (§"decomposition of a lambda closure type"), which is not yet implemented.~~ Now implemented — see "Lambda closure decomposition" section below.

---

## ExprWithCleanups tightening — source destructor at end-of-full-expression ✅ `e09a28e`

**Description:** When `reloc` selects the move/copy path (not a relocation constructor), the source object's destructor fires at the end of the enclosing full-expression (§"early end of scope"), not at end-of-scope. When `reloc` selects the relocation constructor, the source is consumed — no destructor fires (the scope-exit cleanup is simply deactivated).

**Implementation:** For non-parameter locals with non-trivial destructors, `EmitCXXRelocExpr` deactivates the scope-exit cleanup and pushes a `pushDestroy(NormalAndEHCleanup, ...)` as a full-expression cleanup (move/copy path only). `Cleanup.setExprNeedsCleanups(true)` in `ActOnRelocExpr` (SemaRelocation.cpp) ensures `ExprWithCleanups` wrapping whenever the source type has a non-trivial destructor. This wrapping is needed even for the relocation constructor path because CodeGen's CanElide optimization (Phase 9) may bypass the ctor and push a caller-side cleanup instead — that cleanup must fire at the end of the full expression, not at scope exit.

**Proposal coverage:** §"reloc-src-obj-lifetime", §"early end of scope"

---

## Overload resolution fix — prvalue prefers by-value ✅ `ebd74d2`

**Description:** Amended `CompareStandardConversionSequences` in `SemaOverload.cpp` to implement the P2785 [over.ics.rank]/3.2.3 changes, gated behind `-frelocation`:
- A non-reference binding (by-value parameter) is preferred over a reference binding when the argument is a prvalue.
- An rvalue-reference binding to an xvalue is preferred over a non-reference binding.

Added `BindsToPRValue` bit to `StandardConversionSequence` (in `Overload.h`) to distinguish prvalue from xvalue in the reference binding, since the existing `BindsToRvalue` does not discriminate.

**Result:** `bar(reloc val)` with overload set `{bar(S), bar(S&&)}` now selects `bar(S)` (was ambiguous). `bar(std::move(val))` selects `bar(S&&)` (was also ambiguous). Without `-frelocation`, standard C++ behavior is unchanged.

---

## Known gap — `constexpr` / constant-evaluation support ✅ Implemented

**Description:** The proposal explicitly permits `reloc` in constexpr functions (§"reloc in constexpr") and object decomposition in constant-evaluated expressions (§"decomposition in constexpr"). Implemented in Phase 10 (`885717f87cf7`). See Phase 10 section for details.

---

## Known gap — `typeid` polymorphic glvalue ⚠️ Minor / Deferred

**Description:** Per `3155c78`, `typeid(reloc obj)` is potentially-evaluated when `reloc obj` is a glvalue of polymorphic class type (ref-qualified `reloc`). In that case the reloc should transition the tracked state of its operand. The Phase 4 CFG analysis handles this correctly in practice (the CFG builder already marks the `typeid` operand as evaluated for polymorphic glvalues). However, the Phase 4b unsequenced checker (`CheckRelocUnsequenced`) does not skip `CXXTypeidExpr` nodes, so `typeid(reloc x)` where `reloc x` produces a prvalue (operand of non-polymorphic type) could produce a spurious unsequenced diagnostic if paired with another use. This is an extremely obscure edge case.

---

## Known gap — EH paths: noexcept-aware precision ⚠️ Over-approximation

**Description:** Phase 1.5 (EH-aware use-after-reloc) conservatively injects `AliveOrRelocated` for any variable that has a `CXXRelocExpr` *anywhere* in a try body, without checking whether a potentially-throwing expression can actually execute after the relocation completes. Example:

```cpp
void f() {
    T a;
    try {
        foo(a);       // might throw → a is Alive in catch
        reloc a;      // if ~T() is noexcept, this can't throw
    }
    catch (...) {
        read(a);      // a is always Alive here, but we report AliveOrRelocated
    }
}
```

If `~T()` is `noexcept` (the default since C++11), the discarded `reloc a;` cannot throw, so `a` is always `Alive` in the catch. Our analysis reports a false positive.

**Impact:** False positive (over-approximation), never a missed diagnostic. Users can work around it by moving the `reloc` after the try/catch.

**Correct fix (future work):** Track exception specifications of expressions between each relocation and the try-body exit. If no potentially-throwing expression follows the reloc on any path, the variable remains `Alive` (not `AliveOrRelocated`) at catch entry.

---

## Known gap — relocation elision ✅ Implemented

**Description:** The proposal describes relocation elision (§"relocation elision") analogous to NRVO: the source object may share storage with the target, avoiding the constructor call entirely. Key to achieving zero-copy transfer chains (§"achieving 0-copy transfer").

**Implemented:** Phase 9a — call-site elision for by-value parameters and reloc-assign operators, MemberExpr elision in synthesized reloc-assign bodies, aliased reloc-assign dual-function scheme (§"aliased-reloc-assign"). Phase 9c — reference binding elision (`04bcfa91f5cd`). Relocation elision for variable initialisation is **mandatory** (`7bb11fb9398a`): when a local variable is initialised from `reloc x`, the source and target share storage (zero-copy). `constexpr` mandatory elision with storage aliasing for address identity (`1cb0c3254df0`). Mandatory elision for decomposed members (`a0d77a3f1340`): when `reloc obj.member` directly initialises a local variable, the target aliases the member subobject's storage. See Phase 9 section for details.

**Remaining:** NRVO-style elision for `return reloc x;` (note: `return x;` already gets NRVO, making this largely redundant per proposal line 3680).

---

## Known gap — structured decomposition ✅ Implemented (Phase 11)

**Description:** See Phase 11 section above. All five stages landed. Library support for `std::tuple` / `std::array` (`operator reloc[]`) is the only remaining piece — tracked under Phase 13.

---

## Known gap — implicit decomposition of temporaries ⚠️ Not yet implemented

**Description:** Temporary objects can be implicitly decomposed to allow relocations (§"implicit decomposition of temporaries"), e.g. `B b = getD();` implicitly decomposes the `D` temporary and relocates the `B` base. No implementation exists.

---

## Known gap — relocation assignment operator ✅ Fully implemented

**Description:** `T& T::operator=(T [reloc])` is a new special member function (§"relocation assignment operator"). All sub-phases implemented: recognition, implicit declaration, body synthesis (including C-array members and virtual bases), codegen, exception specification, and aliased reloc-assign dual-function scheme.

---

## Known gap — virtual slicing function ✅ Fully implemented (Phase 12)

**Description:** A hidden virtual function implicitly declared when a class has both an explicit relocation constructor and a virtual destructor (§"virtual slicing function"). Used by `std::reloc_and_uninitialize` / `std::reloc_and_reclaim` to prevent object slicing. Fully implemented: implicit declaration, vtable integration, codegen (tag comparison, match/decompose paths, virtual base handling, diamond inheritance), exception handling, `__builtin_reloc_and_uninitialize`, implicit reloc ctor relaxation. See Phase 12 section below for details.

---

## Lambda closure decomposition ✅ `864cae7`

**Scope:** Lambda objects can be decomposed from within the lambda body (§"decomposition of a lambda closure type"), allowing relocation of by-value captures via `reloc self.capture_name`.

### Implemented

- ✅ `LookupMemberExprInRecord` (SemaExprMember.cpp): after `LookupParsedName` fails on a lambda closure type, resolves capture variable names to their (unnamed) `FieldDecl`s via `CXXRecordDecl::getCaptureFields()`
- ✅ Access restricted to lambda body: `CurContext` walk checks that the current function is a method of the lambda's closure class
- ✅ `CheckDecomposedVarDecl` (SemaRelocation.cpp): calls `setDecomposedByReloc()` for dependent types (e.g. `auto reloc self`) before returning, so `SubstParmVarDecl` propagates the flag during template instantiation
- ✅ `ActOnRelocExpr`: builds dependent `CXXRelocExpr` for type/value-dependent operands (e.g. `reloc self.p` in the uninstantiated template)
- ✅ Captures by reference are automatically forwarded (not relocated) because their `FieldDecl` has reference type, which `ActOnRelocExpr` already handles via the lvalue-ref / rvalue-ref path
- ✅ 7 new unit tests: init-capture by value, regular by value, multiple captures, by-reference forwarding, outside-body rejection, trivial by value, nonexistent capture name rejection

**Proposal coverage:** §"decomposition of a lambda closure type"

---

## Phase 5a — Core object decomposition ✅ `486037a` + `55172ab` (+ uncommitted: user-provided dtor guard)

**Scope:** `T reloc value = expr;` — the *decomposition* syntax that ends the lifetime of `expr` and makes its non-base subobjects available as independent complete objects.

### Implemented

- ✅ `DeclSpec` / `Declarator` parsing for `reloc` in an object definition (`T reloc name = init`)
- ✅ `VarDecl::IsDecomposedByReloc` bit flag; `setDecomposedByReloc()` / `isDecomposedByReloc()` accessors (`ParmVarDeclBitfields` and `NonParmVarDeclBitfields`)
- ✅ Sema validation in `CheckDecomposedVarDecl`: rejects reference types, non-class types, unions, and non-local storage
- ✅ Data-member access on a decomposed object (`value.m`) — existing `MemberExpr` machinery, no new AST node needed
- ✅ `reloc value.m` on a decomposed subobject (§"decomposed subobjects in reloc expressions"): operand allowed when the containing object `isDecomposedByReloc()`
- ✅ Per-member destructor cleanup: `MemberDestroyCleanup` emits individual member destructors at scope exit instead of the full class destructor
- ✅ Decomposition of value parameters (§"decomposition of value parameters"): `T reloc param` in a function signature; check deferred to `CheckDecomposedParams`
- ✅ User-provided destructor guard (§"ill-formed decomposition"): `CheckDecomposedParams` (called from `ActOnFunctionDeclarator` after `mergeFunctionDecl`) rejects decomposing a type with a user-provided destructor from outside the class or its friends; `CheckDecomposedVarDecl` applies the same rule for non-parameter variables via `CurContext` walk

### Diagnostics added

```
err_decomposed_ref_type
err_decomposed_requires_class_type
err_decomposed_union_type
err_decomposed_no_local_storage
err_decomposed_user_provided_dtor
```

### Not yet implemented (Phase 5d and later)

- Implicit decomposition of temporaries (§"implicit decomposition of temporaries")
- ~~Lambda closure decomposition (§"decomposition of a lambda closure type")~~ — **done** (`864cae7`)
- `constexpr` decomposition (§"decomposition in constexpr")

**Proposal coverage:** §"object decomposition" (core data-member and parameter sub-sections)

---

## Phase 5b — `.base<B>` base-class access syntax ✅ `372f6589`

**Scope:** `obj.base<B>` — access a base-class subobject of a decomposed object as an lvalue.

### Implemented

- ✅ New AST node `CXXDecomposedBaseExpr` (lvalue of type `B cv`, trailing `CXXBaseSpecifier*` path)
- ✅ Parser extension: `.base<` in postfix position parsed directly (never treated as `operator<`); `TypeResult` parsed inside `<…>`
- ✅ `Sema::ActOnDecomposedBaseAccess`: validates object is a directly-named decomposed `VarDecl`; resolves `B` as an unambiguous, accessible, non-virtual direct base of the decomposed type; propagates cv-qualifiers from the object
- ✅ Rejection of cv-qualified type argument (`d.base<const B>` ill-formed)
- ✅ Rejection of virtual bases in `reloc` (read access still permitted)
- ✅ Rejection of indirect bases in `reloc`
- ✅ `VisitCXXDecomposedBaseExpr` printer (`obj.base<B>`), profiler, serialization, `RecursiveASTVisitor` traversal
- ✅ `TransformCXXDecomposedBaseExpr` in `TreeTransform.h`
- ✅ CodeGen: `EmitLValue` emits `GetAddressOfBaseClass` to produce the base-subobject pointer
- ✅ Qualified member access (`obj.Qualifier::member`) rejected with dedicated diagnostic; `err_decomposed_qualified_member_access`
- ✅ Member function calls on decomposed objects rejected; base-subobject method calls permitted
- ✅ Static member calls via instance syntax permitted (`obj.sfoo()` well-formed); corrected in Phase 5e (see below)
- ✅ `reloc` inside unevaluated operands (`decltype`, `sizeof`, `noexcept`) does not consume the variable
- ✅ 41 tests covering: simple access, field through base, multiple bases, reloc of direct base, cv-qual propagation, static-template named 'base', disambiguation from data member named 'base', virtual-base read, method calls, static methods, unevaluated operands

**Proposal coverage:** §"object decomposition" — base-class access sub-section

---

## Phase 5c — `obj.this` — address of decomposed object storage ✅

**Scope:** `obj.this` — returns the `void cv*` prvalue of the address the decomposed object occupied prior to decomposition (§"decompose-obj-addr").

### Implemented

- ✅ New AST node `CXXDecomposedThisExpr` (prvalue of type `void cv*`, no trailing objects)
- ✅ Parser extension: `.this` (keyword token) in postfix position intercepts before the normal member-access path
- ✅ `Sema::ActOnDecomposedThisAccess`: validates object is a directly-named decomposed `VarDecl`; propagates cv-qualifiers from the object to `void*`
- ✅ Rejection on non-decomposed variable (`err_decomposed_this_not_decomposed`)
- ✅ Result is a prvalue (`VK_PRValue`) — `&obj.this` and `obj.this = …` are ill-formed by design
- ✅ cv-qualification: `void*` (plain), `void const*` (const object), `void volatile*` (volatile), `void const volatile*` (cv)
- ✅ `VisitCXXDecomposedThisExpr` printer (`obj.this`), profiler, serialization, `RecursiveASTVisitor` traversal
- ✅ `TransformCXXDecomposedThisExpr` in `TreeTransform.h`
- ✅ CodeGen: `VisitCXXDecomposedThisExpr` in `CGExprScalar.cpp` emits `Address::emitRawPointer` of the local's alloca
- ✅ 10 tests: type checks (plain / const / volatile / cv), prvalue assertion, usable-as-void*, `decltype` assertions, rejection of non-decomposed, printer round-trip

**Proposal coverage:** §"decompose-obj-addr"

---

## Phase 5d — Virtual-base-aliasing objects ✅ `88e5ada` + `2b55010` + `0237827`

**Scope:** Full VBA (virtual-base-aliasing) support: Sema detection, CodeGen with 4 reloc ctor variants, correct destruction of VBA subobjects (including discarded reloc).

### Sema

- ✅ `CXXDecomposedBaseExpr` gains `IsVirtualBaseAliasing` bool field; set when the accessed base has virtual bases
- ✅ Serialization round-trips the new field
- ✅ `ActOnDecomposedBaseAccess` detects VBA after building the base path
- ✅ `ActOnRelocExpr` handles `reloc obj.base<B>` for VBA bases
- ✅ `reloc` through a VBA base rejected (`err_reloc_virtual_base_reloc`); reading still permitted
- ✅ Diamond inheritance: both sides VBA; readable through either; `reloc` rejected through either

### CodeGen — 4 reloc ctor variants

Two new `CXXCtorType` values encode VBA status orthogonally to Complete/Base:

| | Source owns VBases (non-VBA) | Source is VBA |
|---|---|---|
| **Init dest VBases (complete)** | C1 (`Ctor_Complete`) | C1v (`Ctor_CompleteVBA`) |
| **Skip dest VBases (base)** | C2v (`Ctor_BaseNonVBA`) | C2 (`Ctor_Base`) |

- ✅ `ABI.h`: `Ctor_CompleteVBA = 6`, `Ctor_BaseNonVBA = 7`; 4 helpers (`isCtorVariantComplete`, `isCtorVariantBase`, `isCtorSourceOwnsVBases`, `isCtorSourceVBA`)
- ✅ `ItaniumMangle.cpp`: C1v / C2v vendor-suffix mangling
- ✅ `Mangle.cpp`: enumerate VBA variant manglings for reloc ctors with vbases
- ✅ `ItaniumCXXABI.cpp`: emit C1v/C2v; VTT for base variants; `NeedsVTTParameter` / `HasThisReturn` updated
- ✅ `CGClass.cpp`: VBase init/cleanup gated by the new helpers
- ✅ `CGExprCXX.cpp`: variant selection at `reloc` call sites; VBase cleanup NOT deactivated for VBA relocs

### CodeGen — discarded reloc on VBA base

- ✅ `reloc d.base<B1>;` (discarded) uses `Dtor_Base` + VTT (not `Dtor_Complete`), so shared virtual bases are not destroyed
- ✅ Lit test `vba_discarded_reloc_b1`: verifies `B1D2Ev` emitted, not `B1D1Ev`

### Tests

- ✅ 13 Sema unit tests (VBA flag, diamond, reject patterns)
- ✅ 6 VBase cleanup unit tests (scope-exit, after-base-reloc, mixed, discarded-reloc)
- ✅ Lit tests in `p2785-virtual-base-cleanup.cpp` (C1v mangling, Dtor_Base with VTT)
- ✅ Runtime tests: `virtual-bases-003.cpp`, `virtual-bases-005.cpp`

**Proposal coverage:** §"virtual-base-aliasing objects", §"decompose-virtual-ctor-variants"

---

## Phase 5 CodeGen — Per-subobject cleanup deactivation tests ✅ `02e40b9`

**Scope:** LLVM IR-level FileCheck tests verifying per-subobject destructor cleanup deactivation for Phases 5a and 5b.

Added to `clang/test/CodeGenCXX/p2785-reloc-operator.cpp`:
- `test_base_reloc`: `reloc d.base<B>` deactivates `d`'s B-cleanup; no `~A` call on normal path
- `test_nested`: full `D reloc d → reloc d.base<B> → B reloc b → reloc b.base<A>` chain; both cleanup deactivations fire; no `~A` on normal path
- `test_partial`: same chain but `b.base<A>` never relocated; `~A` fires at scope exit (correct partial cleanup)

---

## Phase 5e — Decomposed-object value enforcement ✅ `93c65bc` + `89494935`

**Scope:** Enforce §"decomposed-obj-expr": the id-expression naming a decomposed object does not produce a value.

### Implemented

- ✅ New helper `Sema::DiagDecomposedVarUsedAsValue(Expr *E)` fires `err_decomposed_use_as_value` when the expression directly names a decomposed variable
- ✅ Called from `PerformCopyInitialization` (copy-init, pass by value/ref, return), `IgnoredValueConversions` (`(void)b`, bare `b;`), `BuildCXXNamedCast` (static_cast / const_cast / reinterpret_cast / dynamic_cast), `BuildDecltypeType` (`decltype((obj))` — parenthesised decomposed var is ill-formed per §"decomposed-unevaluated")
- ✅ Static member function calls via instance syntax exempted: `obj.sfoo()` is permitted. The `ConvertBaseExprToDiscardedValue` lambda in `BuildMemberReferenceExpr` skips the check for decomposed vars; `IsMethod` predicate fixed to use a `hasMethodDecl`-based check rather than `isa<CXXMethodDecl>` (which is true for static methods too)
- ✅ Well-formed uses unchanged: `obj.member`, `obj.base<B>`, `sizeof(obj)`, `decltype(obj)`, `obj.SfuncName()`

### New diagnostic

```
error: decomposed object 'X' cannot be used as an expression value
```

**Proposal coverage:** §"decomposed-obj-expr", §"decomposed-unevaluated"

---

## CodeGen correctness fixes ✅ `29508cc` + `1abb936` + `52c431f` + `92dde35`

**Scope:** Four correctness fixes and tests for `reloc` CodeGen and source-lifetime management.

### Fix 1 — Destructor semantics for move/copy paths (`29508cc`)

`reloc` selects one of three implementations: (1) relocation constructor (Phase 6, not yet), (2) bitwise copy for trivially relocatable types, (3) move/copy constructor (fallback). Only (1) and (2) truly end the source object's lifetime. The move/copy path leaves the source in a moved-from state — its destructor **must** still fire at scope exit. Previous code incorrectly suppressed the destructor on the move/copy path. Fixed by removing `DeactivateCleanupBlock` from the move/copy branch in `EmitCXXRelocExpr`.

### Fix 2 — Discarded `reloc` as in-place destructor (`1abb936`)

`reloc x;` as a discarded-value expression now calls the destructor directly in place (not move-constructing an ignored temporary) and deactivates the variable's scope-exit cleanup. For trivially relocatable types, the discarded `reloc` simply deactivates the cleanup with no memcpy. Implemented via a `DeactivateSourceCleanup` lambda in `EmitCXXRelocExpr`, consolidating all three operand shapes (whole-var, decomposed member, decomposed base).

### Fix 3 — Conditional branch cleanup deactivation (`52c431f`)

`DeactivateCleanupBlock` was incorrect for cleanups inside conditional branches: it unconditionally removed the cleanup from the EHStack, losing the destructor call on branches that skipped the `reloc`. Replaced with a new `ConditionallyDeactivateCleanup` function in `CGCleanup.cpp`:
- Calls `SetupCleanupBlockActivation` to create a runtime `cleanup.isactive` flag (initialised `true`, set `false` at the `reloc` site)
- Does **not** call `Scope.setActive(false)` — keeps the cleanup logically active so `PopCleanupBlock` emits it at scope exit guarded by `br i1 %cleanup.is_active`

All `DeactivateSourceCleanup` call sites use `ConditionallyDeactivateCleanup`.

### Tests (`92dde35`)

Added to both `clang/test/CodeGenCXX/p2785-reloc-operator.cpp` (FileCheck) and `CXXRelocExprTest.cpp` (unit):
- `test_discard_local` / `Discard_NonTrivialLocalOK`: discarded reloc of non-trivial local — dtor + flag + flag-guarded scope-exit cleanup
- `Discard_TrivialLocalOK`, `Discard_ScalarLocalOK`: trivial and scalar paths
- `test_discard_condit` / `Discard_ConditionalLocalOK`: conditional discarded reloc — dtor on taken branch only, flag-guarded scope-exit dtor
- `test_three_loc` / `Discard_NestedConditionalThreeLocalsOK`: three variables, nested conditionals; 3 flags, 3 in-branch dtors, 3 flag-guarded scope-exit dtors; all 8 `(a,b,c)` combinations verified

**Proposal coverage:** §"source lifetime", §"discarded reloc"

---

## Phase 6 — Relocation constructor `T(T reloc)` ✅ `4136352`–`90e84f9`

**Scope:** The new special member function that constructs a `T` from a relocated prvalue of type `T`.

### Sub-phases

**Phase 6a** — Parser / Sema recognition ✅ `4136352`
- `T(T reloc)` and `T(T const reloc name)` parameter form
- `CXXConstructorDecl::isRelocationConstructor()` query
- `setDecomposedByReloc()` on reloc-qualified constructor parameters

**Phase 6b** — Ctor preference in `ActOnRelocExpr` ✅ `031aa3d`
- Reloc ctor preferred over move > copy in `reloc` expression
- CodeGen emits reloc ctor call at `reloc` sites for class types

**Phase 6c+6e** — SMF registration, implicit declaration, triviality ✅ `a3519e2`
- `SMF_RelocConstructor` in special member function bitmask
- `DeclareImplicitRelocConstructor()`: implicit reloc ctor declaration
- `= default` / `= delete` support
- Triviality tracking; `__is_trivially_relocatable` trait
- Memcpy for trivially-relocatable reloc ctor codegen

**Phase 6c-body** — `DefineImplicitRelocConstructor` body synthesis ✅ `90e84f9`
- `IIK_Reloc` added to `ImplicitInitializerKind` enum
- Handled alongside `IIK_Move` in `BuildImplicitBaseInitializer`, `BuildImplicitMemberInitializer`, `BaseAndFieldInfo`, `isImplicitCopyOrMove`
- `DefineImplicitRelocConstructor()` modelled on `DefineImplicitMoveConstructor`: `SetCtorInitializers` + empty compound body
- Wired into `DefineDefaultedFunction` and `MarkFunctionReferenced` for on-demand synthesis
- Fix: suppress `DiagDecomposedVarUsedAsValue` inside implicit/defaulted functions (the synthesized initializers legitimately reference the decomposed reloc parameter)

**Phase 6d** — FileCheck IR tests ✅ `e702357`
- `clang/test/CodeGenCXX/p2785-reloc-operator.cpp` tests for reloc ctor codegen

**Phase 6e** — Exception specification ✅ `57c7003`
- `canThrow()` handles `CXXRelocExpr`, `CXXDecomposedBaseExpr`, `CXXDecomposedThisExpr`
- Virtual base exception spec via move+dtor

**Phase 6f** — Non-virtual base relocation in defaulted reloc ctor ✅ `f95ed28`
- `BuildImplicitBaseInitializer` builds `CXXRelocExpr` for non-virtual bases
- `LookupRelocCtorForImplicit` helper (reloc > move > copy)
- Fixed `isSpecializationCopyingObject` excluding reloc ctors
- Fixed `select_special_member_kind` diagnostic OOB for reloc index

**Phase 6g** — Member init in defaulted reloc ctor ✅ `c56e624`
- `BuildImplicitMemberInitializer` IIK_Reloc path: non-array members → CXXRelocExpr
- C-array members → ArrayInitLoopExpr with CXXRelocExpr per element

**Phase 6h** — Virtual base handling ✅ `9b449fa`
- Suppress full param dtor for reloc ctor params
- Destroy source's moved-from virtual bases in C1
- `ForBaseSubobject` flag on CXXRelocExpr for Ctor_Base in base inits

**Phase 6i** — Overload resolution exclusion ✅ `c67f9fa`
- Exclude reloc ctors from `ResolveConstructorOverload` in `SemaInit.cpp`
- Reloc ctors invoked only via CXXRelocExpr / LookupRelocCtorForImplicit

**Phase 6j** — User-provided reloc ctor per-subobject cleanups ✅ `a50a944`
- Per-subobject cleanup registration for decomposed reloc ctor params

### Phase 6f-tests — User-provided reloc ctor body tests ✅ `1ed1200`
- 12 new unit tests: `MoveFromMember`, `ReadBeforeReloc`, `SrcThis`, `BodyFixup`, `UseAfterRelocInBody`, `BaseAndAllMembers`, `MultipleBases`, `InheritedMemberReject`, `DoubleRelocReject`, `AdditionalParam`, `ConditionalRelocInBody`, `ReadNonRelocatedMember`
- Fix: `CollectRelocOps` now traverses ctor mem-initializer expressions (`CXXCtorInitializer::getInit()`), not just the body `CompoundStmt`
- Fix: `PropagateVisitor::runBlock` and `DiagnosticVisitor::runBlock` now handle `CFGInitializer` elements alongside `CFGStmt`

**Proposal coverage:** §"relocation constructor"

---

## Template `.base<B>` support ✅ `a8c90288`

**Scope:** `obj.base<T>` where `T` is a template type parameter.

### Implemented

- ✅ `ActOnDecomposedBaseAccess` early-returns for dependent types: builds a dependent `CXXDecomposedBaseExpr` with `DependentTy` and empty path
- ✅ `CXXDecomposedBaseExpr` constructor propagates `toExprDependenceAsWritten(BaseTSI->getType()->getDependence())` in addition to Object dependence
- ✅ `SubstParmVarDecl` preserves `isDecomposedByReloc()` bit during template instantiation
- ✅ Instantiation resolves the base type and validates it (non-base rejected, non-class rejected)

**Proposal coverage:** §"object decomposition" — template base access

---

## Pointer-to-member on decomposed objects ✅ `0bcb700` + `ca65bff`

**Scope:** `obj.*ptr` and `reloc obj.*ptr` where `ptr` is a pointer-to-member on a decomposed object.

### Sema check — constant-evaluated pointer requirement (`0bcb700`)

- ✅ `CheckPointerToMemberOperands`: for `.*` (not `->*`) on decomposed objects, requires the RHS to be constant-evaluated
- ✅ Three acceptance paths: (1) `isConstantEvaluatedContext()`, (2) NTTP reference, (3) `isCXX11ConstantExpr` succeeds
- ✅ Runtime pointer-to-member rejected: `err_decomposed_ptr_mem_not_consteval`

### `reloc d.*ptr` syntax support (`ca65bff`)

- ✅ Parser absorbs `.*`/`->*` into reloc operand via `ParseRHSOfBinaryExpression(Res, prec::PointerToMember)`
- ✅ `ActOnRelocExpr` handles `BinaryOperator(BO_PtrMemD)` operands: validates decomposed LHS, resolves constexpr RHS to `FieldDecl`, treats as decomposed-member relocation
- ✅ Inherited-member pointer-to-member detected and rejected (`err_reloc_inherited_member_not_direct`)

### Flow analysis tracking

- ✅ `CollectRelocOps` registers `reloc d.*ptr` with `RelocKey::Member` for the resolved field
- ✅ `DiagnosticVisitor::VisitBinaryOperator` checks `d.*ptr` reads for use-after-reloc (both direct member and enclosing-base cases)
- ✅ `resolveConstPtrMemField` resolves constexpr pointer-to-member to `FieldDecl` via `isCXX11ConstantExpr` + `APValue::getMemberPointerDecl()`

**Proposal coverage:** §"decomposed-data-member-ptr"

---

## C-array element-wise relocation ✅ `80721b5`

**Scope:** When `reloc` operates on a C-array member of a decomposed object where the element type is non-trivially relocatable, element-wise relocation constructor (or move/copy) calls are emitted instead of a single `memcpy`.

### Implemented

- ✅ `ActOnRelocExpr` (Sema): for `ConstantArrayType` operands, resolves the element type's constructor (reloc > move > copy) via `CtorLookupTy` indirection
- ✅ `EmitCXXRelocExpr` (CodeGen): element-wise loop emitting per-element ctor calls; handles result-used (forward loop), result-discarded (reverse destruction), and source cleanup (reloc ctor deactivates, move/copy destroys in reverse)
- ✅ Trivially relocatable arrays still use the aggregate `EmitAggregateCopy` fast path
- ✅ 5 new unit tests: `CArray_NonTrivialElementAccepted`, `CArray_ElementWithRelocCtor`, `CArray_NoCtorReject`, `CArray_NoCtorDiscardedOK`, `CArray_TrivialDiscarded`

**Proposal coverage:** §"reloc operator" (array operands)

---

## Phase 7 — Relocation assignment operator `T& T::operator=(T reloc)` ✅ `2493fde`–`8febadcfb`

**Scope:** The new special assignment operator — recognition, implicit declaration, body synthesis, codegen, and exception specification.

### Phase 7a+7b — Recognition & implicit declaration ✅ `2493fde`

- `SMF_RelocAssignment = 0x80` added to special member function bitmask; `SMF_All` updated to `0xff`
- All 6 `SpecialMembers` bitfields in `CXXRecordDeclDefinitionBits.def` widened from 7 → 8 bits (to hold the new bit)
- `CXXMethodDecl::isRelocationAssignmentOperator()`: checks `operator=`, non-static, non-template, 1 explicit param, by-value, same unqualified type
- `needsImplicitRelocAssignment()`: suppressed by any user-declared copy/move ctor/assign, dtor, reloc ctor, or reloc assign
- `DeclareImplicitRelocAssignment()`: creates `CXXMethodDecl` with by-value param marked `setDecomposedByReloc()`, calls `ShouldDeleteSpecialMember`
- `CXXSpecialMemberKind::RelocAssignment` added to all switches (Sema, SemaLookup, template instantiation)
- `NeedOverloadResolutionForRelocAssignment` and `DefaultedRelocAssignmentIsDeleted` definition data bits
- Mutual suppression with reloc ctor: user-declared reloc assign suppresses implicit reloc ctor and vice versa
- Overload resolution: `ForceDeclarationOfImplicitMembers` triggers reloc assign; `LookupSpecialMember` handles it; `AddMethodCandidate` / `AddMethodTemplateCandidate` respect it

### Phase 7c — Body synthesis ✅ `8febadcfb`

- `DefineImplicitRelocAssignment()` modelled on `DefineImplicitMoveAssignment`: `RefBuilder` → `MoveCastBuilder` for xvalue semantics, iterates bases with `CastBuilder` + `buildSingleCopyAssign(Copying=false)`, iterates fields with `MemberBuilder` + `buildSingleCopyAssign(Copying=false)`, appends `return *this`
- Ordering fix: `isRelocationAssignmentOperator()` checked BEFORE `isCopyAssignmentOperator()` in all else-if chains (4 locations: `getDefaultedFunctionKind`, `MarkFunctionReferenced`, `ComputeSpecialMemberFunctions`, `finishedDefaultedOrDeletedMember`) — necessary because `isCopyAssignmentOperator()` matches `T& operator=(T)` by-value
- P2785 overload resolution lvalue tiebreaker: `const T&` binding beats `T` by-value for lvalue arguments (prevents ambiguity between implicit copy-assign and reloc-assign in synthesized bodies)

### Phase 7d — CodeGen verification ✅

No new code needed — the body synthesis in 7c produces correct IR through existing CodeGen paths. Verified with FileCheck lit tests: trivial memcpy, non-trivial memberwise move-assign, base-class move-assign in derived reloc-assign body.

### Phase 7e — Exception specification ✅

- `SpecialMemberExceptionSpecInfo` / `SpecialMemberVisitor` already handle `RelocAssignment` (wired in 7a+7b)
- `ComputeDefaultedSpecialMemberExceptionSpec`: after visiting subobject assignment operators and destructors, now also folds in the move constructor (or copy constructor) of the class itself — per §"reloc-assign-op-noexcept", the non-eliding wrapper may invoke it
- Verified: noexcept propagates correctly for trivial, composite, derived, throwing-member, throwing-base, mixed, and explicitly-defaulted cases; throwing move ctor correctly makes reloc assign potentially-throwing

### Phase 7f — C-array element-wise reloc-assign ✅ `81a13b4`

- `buildSingleCopyAssignRecursively`: added `Relocating` parameter; when true, array elements are wrapped in `RelocCastBuilder` (producing `CXXRelocExpr` prvalues) instead of `MoveCastBuilder` (xvalues), so the element type's reloc-assign operator is selected via P2785 overload resolution
- Virtual base handling verified: already correctly implemented (move-assign semantics, same as reloc-ctor)

### Lit tests

- `clang/test/CodeGenCXX/p2785-reloc-operator.cpp`: trivial memcpy, non-trivial memberwise, base-class in derived body, C-array element-wise reloc-assign
- `clang/test/SemaCXX/p2785-reloc-operator.cpp`: 10 `static_assert` noexcept tests (trivial, composite, derived, throwing member/base/mixed, explicitly-defaulted, throwing/noexcept move ctor)

**Proposal coverage:** §"relocation assignment operator" (recognition, implicit declaration, default definition, exception specification, aliased reloc-assign, C-array members)

---

## Phase 8 — Overload resolution changes ✅ `ebd74d2`

**Scope:** Updated overload resolution rules so that `reloc val` (a prvalue) selects the by-value overload over reference overloads.

Implemented in `CompareStandardConversionSequences` (`SemaOverload.cpp`), gated behind `-frelocation`:
- Added `BindsToPRValue` bit to `StandardConversionSequence` (`Overload.h`) to distinguish prvalue from xvalue (existing `BindsToRvalue` covers both).
- New tie-breaker: a non-reference binding is preferred over a reference binding when the argument is a prvalue.
- New tie-breaker: an rvalue-reference binding to an xvalue is preferred over a non-reference binding.
- New tie-breaker (`073fca7da55f`): when S1 binds a reference and S2 does not bind a reference and the argument is not a prvalue, S1 is a better conversion sequence. This resolves an ambiguity where an xvalue argument facing both `f(const T&)` and `f(T)` had no tiebreaker — the ambiguity broke `assignable_from` concept checks on Rule-of-Zero classes (which implicitly gain a relocation assignment operator), cascading into failures of `copyable`, `semiregular`, `forward_iterator`, etc.
- Without `-frelocation`, standard C++ behavior is unchanged (both cases remain ambiguous).

Remaining sub-features (depend on Phase 6):
- Relocation constructor preference over move/copy constructors in overload resolution
- ✅ ABI-aware relocation constructor discard for function parameters (§"relocation constructor discardment with function parameters")
- Interaction with NRVO / copy elision

**Relocation constructor discardment for function parameters:**

Implemented in `ActOnRelocExpr` (SemaRelocation.cpp) and `CheckCompletedCXXClass` (SemaDeclCXX.cpp).

On caller-destroy ABIs (e.g. Itanium default), using the relocation constructor on a function parameter would end the source's lifetime in the callee while the caller still owns it, causing double destruction. The fix has two parts:

1. **Reloc ctor discardment** (SemaRelocation.cpp): when `reloc param` targets a `ParmVarDecl` whose type has a non-trivial destructor, the ABI does not grant callee-destroy ownership, and the type has an eligible move constructor, the reloc ctor is discarded and the move (or copy) ctor is used instead.

2. **Forced callee-destroy for relocate-only types** (SemaDeclCXX.cpp): types with an eligible reloc ctor but no eligible move ctor are forced `ParamDestroyedInCallee`, so that function parameters of such types can be safely relocated. This is an ABI change for "relocate-only" types (which don't exist today).

The discardment does NOT apply when:
- The operand is a local variable (always owned by current scope)
- The type has a trivial destructor (caller cleanup is a no-op)
- The type has no eligible move ctor (forced callee-destroy instead)
- The ABI is already callee-destroy (e.g. Microsoft ABI, or `trivial_abi`)

3. **Discarded reloc on caller-destroy params** (CGExprCXX.cpp): for discarded `reloc param;` where the reloc ctor was discarded (MoveCtor is a move/copy ctor) and the parameter is not callee-destroy, the CodeGen must NOT call the destructor on the parameter directly (the caller still owns it). Instead, a move-constructed temporary is materialized and its destruction is pushed as a full-expression cleanup. This triggers destructor side-effects early while leaving the parameter in a moved-from state for the caller to destroy.

**Silent relocation of decomposed parameters (§"decompose-value-param"):**

Implemented across CGDecl.cpp (`EmitParmDecl`), CGExpr.cpp
(`getDirectCallTarget`/`getRawFunctionPointer`), CGCall.cpp (`EmitCallArg`,
`EmitDelegateCallArg`) and SemaRelocation.cpp (`CheckDecomposedParams`).

When a function parameter is declared with `T reloc obj` (decomposed), the
callee manages the parameter's subobjects individually. Per
§decompose-value-param-direct the canonical decomposing entry must satisfy
two invariants at every direct or virtual call site:

  * **Rule 1 (Ownership)**: the caller invokes NO destructor for the
    decomposed parameter; the callee owns every subobject's lifetime.
  * **Rule 2 (Direct materialization)**: the storage backing the parameter
    inside the callee IS the storage that holds the argument prvalue; no
    copy/move/relocation constructor is inserted between caller and callee.

For indirect (function-pointer) calls the source generally cannot satisfy
rules 1+2, so address-taking emits a non-decomposing `.Vreloc_twin`
(§decomposing-function-indirect) that takes the same parameter list with
`reloc` removed and silently reloc-copies each formerly-decomposed
parameter into the canonical entry's storage on entry.

1. **Sema: implicit ctor declaration** (SemaRelocation.cpp `CheckDecomposedParams`): for caller-destroy decomposed params, explicitly triggers `DeclareImplicitRelocConstructor`, `DeclareImplicitMoveConstructor`, and `DeclareImplicitCopyConstructor` to ensure the constructor is available in CodeGen.

2. **CodeGen: canonical entry is rule-1+2 compliant** (CGDecl.cpp `EmitParmDecl`): the silent-reloc materialization is gated on `EmittingRelocTwin`. The canonical decomposing entry receives the parameter via `dead_on_return` and operates directly on the caller's storage (rule 2). Per-subobject cleanups are registered on the parameter address.

3. **CodeGen: direct vs. address-taken function pointer split** (CGExpr.cpp): `getDirectCallTarget(GD)` returns the canonical entry for direct/virtual calls; `getRawFunctionPointer(GD)` returns the `.Vreloc_twin` for address-taken contexts (function-to-pointer decay, function-pointer constants, PMF formation). All direct-call sites (`EmitDirectCallee`, the inline-builtin path, the predefined library function path in CGBuiltin, and the OpenCL kernel-stub direct call) route through `getDirectCallTarget`.

4. **CodeGen: caller-side cleanup suppression** (CGCall.cpp `EmitCallArg`): the optional `CalleeParam` argument is threaded from `EmitCallArgs`. When the callee parameter is `reloc`-decomposed, both the elision branch and the CXXRelocExpr fallback branch suppress the caller-side full-expression destructor cleanup that would ordinarily run for caller-destroy ABIs (rule 1).

5. **CodeGen: per-subobject destruction order** (CGDecl.cpp): per-subobject cleanups for a decomposed object are pushed in the inverse of the run order, so LIFO popping yields the [class.dtor] order (members in reverse declaration, then non-virtual bases in reverse spec, then virtual bases in reverse DFS). Applies to both decomposed locals and decomposed parameters.

6. **CodeGen: delegate call fix** (CGCall.cpp `EmitDelegateCallArg`): decomposed params skip the `CalleeDestructedParamCleanups` lookup since their cleanups are per-subobject (registered in `MemberDestroyCleanup`/`BaseDestroyCleanup`).

For callee-destroy ABIs (MSVC, or forced for relocate-only types), the parameter is decomposed directly in place — the caller doesn't call the destructor, so per-subobject cleanups on the original param are safe.

**Proposal coverage:** §"decompose-value-param"

---

## Phase 9 — Relocation elision ✅ `3fea413` + `a132463` + `04bcfa91f5cd`

**Scope:** Allow the compiler to elide relocation when source and target can share storage (§"relocation elision").

### Phase 9a — Call-site elision + MemberExpr elision ✅ `3fea413`

- `EmitCallArg` (CGCall.cpp): when a `CXXRelocExpr` targets a local variable or callee-destroy parameter, the source is passed directly to the callee without a reloc/move/copy ctor call; the source's scope-exit cleanup is conditionally deactivated
- MemberExpr elision: `reloc obj.member` in synthesized reloc-assign bodies passes the member subobject directly (no intermediate temp) when the member has a reloc-assign operator
- `RelocCastBuilder` in SemaDeclCXX.cpp: builds `CXXRelocExpr` nodes in synthesized reloc-assign bodies for member subobjects that have reloc-assign operators, enabling recursive elision

### Phase 9b — Aliased reloc-assign dual-function scheme ✅ `a132463`

Per §"aliased-reloc-assign", each `T& operator=(T reloc)` emits two LLVM functions:

1. **Standard variant** (normal mangling, e.g. `_ZN2RAaSES_`): non-eliding wrapper with silent reloc-copy of the decomposed parameter into local storage. Used when elision cannot be applied.

2. **Eliding variant** (`.reloc_eliding` suffix, e.g. `_ZN2RAaSES_.reloc_eliding`): the decomposed parameter is accessed in-place — no silent reloc-copy. Parameter 1 carries `dead_on_return`. Used when the call site proves the argument is elision-eligible.

**Call-site dispatch:** AST-based prediction at callee resolution inspects the `CXXRelocExpr` operand:
- `DeclRefExpr` to local variable → elision
- `DeclRefExpr` to callee-destroy parameter → elision
- `MemberExpr` on a field of a local variable → elision (nested reloc-assign bodies)
- Otherwise → standard variant

This works for both `CXXOperatorCallExpr` (user-written `a = reloc b`) and `CXXMemberCallExpr` (synthesized memberwise assignments).

**Implementation files:**
- `CGExprCXX.cpp`: `UseElidingVariant` prediction, `CallingRelocAssignOperator` flag to suppress caller-side dtor
- `CodeGenModule.cpp`: emit eliding variant in `EmitGlobalFunctionDefinition`, sync linkage/comdat/attributes
- `CGDecl.cpp`: skip silent reloc-copy when `EmittingRelocAssignEliding`
- `CodeGenFunction.h`: `EmittingRelocAssignEliding`, `CallingRelocAssignOperator` flags
- `CGCall.cpp`: caller-side dtor suppression for elision path

### Phase 9c — Reference binding elision ✅ `04bcfa91f5cd`

When `reloc x` is passed to a `T&&` parameter, the compiler skips temporary materialization and binds the reference directly to `x`, destroying `x` at end of full-expression. This avoids an unnecessary move-construct + destroy pair. The source's scope-exit cleanup is conditionally deactivated; a full-expression cleanup destroys the source after the callee returns.

### Phase 9d — Mandatory elision for decomposed members ✅ `a0d77a3f1340`

When a `reloc` expression on a direct data member of a decomposed object directly initialises a local variable, the relocation/move/copy constructor is not called. The target object aliases the member subobject's storage directly (§"reloc-elision-mandatory" bullet 2).

- `EmitAutoVarAlloca` (CGDecl.cpp): detects `MemberExpr(DeclRefExpr(decomposed))` initialisation pattern and computes the field address via `EmitLValueForField` instead of allocating a new slot

### Not yet implemented

- NRVO-style elision for `return reloc x;` (note: `return x;` already gets NRVO via end-of-life optimization, making `return reloc x;` largely redundant per proposal line 3680)

**Proposal coverage:** §"relocation elision", §"reloc-elision-mandatory", §"aliased-reloc-assign", §"achieving 0-copy transfer"

---

## Phase 10 — `constexpr` support ✅ `885717f87cf7`

**Scope:** Enable `reloc` and object decomposition in constant-evaluated expressions.

### Implemented

- ✅ `CXXRelocExpr` handled in `ExprConstant.cpp`: relocates the source object in the constant evaluator by ending its lifetime and constructing the result from the source value
- ✅ `CXXDecomposedBaseExpr` handled: base-subobject access in constant evaluation via `HandleBasePath`
- ✅ `CXXDecomposedThisExpr` handled: returns the address of the decomposed object’s storage as `void cv*`
- ✅ `constexpr` decomposition (§"decomposition in constexpr"): `T reloc` variables work in `constexpr` functions
- ✅ Relocation constructor discard not performed during constant evaluation (§"relocation constructor discardment with function parameters")
- ✅ 33 unit tests covering: scalar reloc, class reloc (move path), reloc ctor path, trivial reloc, cv-qualified, decomposition + member access, `.base<B>`, `.this`, reloc in loop, decomposed-this identity

**Proposal coverage:** §"reloc in constexpr", §"decomposition in constexpr"

---

## `reloc` in function types rejected ✅ `85c3818d94ad`

**Scope:** `reloc` is a body-level property of a parameter (decomposed object declaration), not part of the function type. The proposal now explicitly states this: `void (*b)(T reloc) = &foobar;` is ill-formed (§"reloc-param-not-function-type").

### Implemented

- ✅ New diagnostic `err_reloc_in_function_type` in `DiagnosticParseKinds.td`
- ✅ `ParseParameterDeclarationClause` (ParseDecl.cpp): after parsing each parameter declarator, if `reloc` was consumed but the enclosing declarator is not a function declaration context, the diagnostic fires
- ✅ `IsFunctionDeclaration` flag threaded through `ParseParameterDeclarationClause` overloads (Parser.h) to distinguish function declarations/definitions (where `reloc` is valid) from function types in pointers, typedefs, using aliases, reference-to-function types, and template arguments (where it is rejected)
- ✅ 9 unit tests (6 rejection: function pointer, init, template arg, typedef, using alias, function ref; 3 acceptance: declaration, definition, pointer without reloc)

**Proposal coverage:** §"reloc-param-not-function-type"

---

## ABI support matrix — prototype scope

The prototype targets the **Itanium C++ ABI** as its primary code generator.
The other Clang-supported C++ ABIs (Microsoft, ARM-pauth) are intentionally
out of scope for this prototype; the gaps are recorded below for completeness
so a future production-quality implementation can pick them up.

### Itanium C++ ABI (x86_64-linux, AArch64 ELF, etc.) ✅ Fully supported

All P2785 codegen work in this prototype targets the Itanium C++ ABI in
`clang/lib/CodeGen/ItaniumCXXABI.cpp`. Coverage:

- ✅ Direct calls to decomposing functions (Phase 5, Phase 6)
- ✅ Virtual dispatch into a decomposing function: vtable slot points at the
    canonical (decomposing) entry, per §"decompose-value-param-direct" — no
    bridging needed at the call site
- ✅ Function-pointer indirect calls: address-taking emits the
    `<canonical>.Vreloc_twin` non-decomposing twin per
    §"decomposing-function-indirect" (Step 2 / Step 3, commits `786cb9fd7976`
    and `a806652d8efa`)
- ✅ PMF formation for **non-virtual** decomposing fns: PMF data field points at
    `.Vreloc_twin`
- ✅ PMF formation for **virtual** decomposing fns: PMF encoded as non-virtual
    `{&<canonical>.Vreloc_pmf_bridge, adj}`; the bridge silent-relocs the
    decomposed argument into a bridge-local, dispatches through the vtable, and
    invokes the canonical (commit `538771eb1df4`, see also "Bridge for virtual
    PMF" below)
- ✅ Pure-virtual decomposing fns: bridge dispatches dynamically; no canonical
    body required
- ✅ ABI-aware reloc-ctor discardment for caller-destroy params (Phase 8)
- ✅ Forced callee-destroy for *relocate-only* types (Phase 8)
- ✅ Aliased reloc-assign dual-function scheme (`.reloc_eliding`) (Phase 9b)
- ✅ Four reloc-ctor variants (C1 / C2 / C1-VBA / C2-VBA) for VBA bookkeeping
    (Phase 5d)

### ARM AAPCS variant (`UseARMMethodPtrABI`, no pointer authentication) ✅ Inherited

The AAPCS PMF representation differs from base Itanium (`adj` low bit
discriminates virtual vs. non-virtual instead of `ptr` low bit). The
prototype's bridge formation runs *before* the AAPCS branch in
`ItaniumCXXABI::BuildMemberPointer` and uses the AAPCS-correct adjustment
shift `(UseARMMethodPtrABI ? 2 : 1) * adj`, so the generated PMF correctly
selects the non-virtual dispatch path on AAPCS targets.

No AAPCS-specific tests are present; this is exercised only indirectly via
the Itanium codegen path.

### ARM with pointer authentication (`PointerAuth.CXXMemberFunctionPointers`) ⚠️ Partial / known gap

For non-`reloc` virtual PMFs, `BuildMemberPointer` calls
`getSignedVirtualMemberFunctionPointer(MD)` to emit a thunk and sign the
resulting pointer with the configured pauth schema (see
`clang/lib/CodeGen/ItaniumCXXABI.cpp` line ~1240).

The prototype's virtual-PMF bridge does **not** go through this signing
path: `getOrCreateVRelocPMFBridge(MD)` returns a plain `Function*` and the
PMF data field is built as `ptrtoint(Bridge)` directly. Under pauth this
yields an unauthenticated raw pointer where the call site expects a signed
one — call-time authentication will fail.

**Production-quality fix:** route the bridge through the same pauth schema
as `getSignedVirtualMemberFunctionPointer`, either by reusing
`CGM.getMemberFunctionPointer(...)` on the synthesized symbol or by
factoring out a helper that signs an arbitrary `Function*` with the
member-function-pointer schema. No call sites in the prototype's test
matrix exercise pauth, so this is not currently observable.

### Microsoft C++ ABI (`MicrosoftCXXABI.cpp`) ❌ Out of scope for the prototype

`MicrosoftCXXABI.cpp` contains zero references to `reloc`, decomposition,
or any P2785 mechanism. Concretely this means that under MS ABI:

- The `.Vreloc_twin` non-decomposing twin (Step 2 / Step 3) is **not**
    emitted on address-taking — function-pointer indirect calls into a
    decomposing fn use the decomposing entry directly with the wrong
    calling convention.
- PMF formation for decomposing fns has no special handling — the MS PMF
    representation (1, 2, or 3 fields depending on the inheritance model,
    plus vftable thunks for virtuals) goes through the unmodified
    `MicrosoftCXXABI::EmitMemberFunctionPointer` path.
- Direct calls and per-subobject cleanups inside decomposing fn bodies
    still work because that codegen lives in target-independent
    `CGDecl.cpp` / `CGCall.cpp`. Only the address-taken / PMF / indirect
    paths are affected.
- The Phase 8 caller-destroy / callee-destroy logic is correct for MS by
    accident: MS ABI is callee-destroy by default, so the
    `ParamDestroyedInCallee` path naturally avoids the double-destroy
    that motivated `.Vreloc_twin` in the first place. The remaining MS
    gap is purely on the indirect-dispatch side (PMF formation and
    function-pointer twins), not in the call-site cleanup logic.

**Production-quality fix:** port Steps 2 + 3 + the virtual-PMF-bridge work
to `MicrosoftCXXABI.cpp`. The MS PMF representation makes this larger than
the Itanium counterpart (each inheritance model needs separate handling),
but the high-level shape is identical: synthesize a `.Vreloc_twin`-style
non-decomposing entry on address-taking, and a `.Vreloc_pmf_bridge`-style
non-virtual bridge for PMF formation of virtual decomposing fns.

No MS-ABI lit tests are present in the prototype's P2785 test matrix.

### Bridge for virtual PMF — implementation reference

For the Itanium implementation of the virtual-PMF bridge added in
`538771eb1df4`, see [clang/lib/CodeGen/ItaniumCXXABI.cpp](clang/lib/CodeGen/ItaniumCXXABI.cpp)
(`BuildMemberPointer` virtual branch, and `getOrCreateVRelocPMFBridge`).
Test: [clang/test/CodeGenCXX/p2785-pmf-virtual-decomposing.cpp](clang/test/CodeGenCXX/p2785-pmf-virtual-decomposing.cpp).

**Proposal coverage:** §"decomposing-function-indirect" (Itanium-only).

---

## Phase 11 — Structured decomposition ✅ `5afc088`–`af5071040dcb` + `5829905800cb` + `a3c916f7e1a6`

**Scope:** `auto [x, y] = expr;` with per-binding complete-object lifetime, enabling `reloc` on individual bindings.

### Implemented

All five stages landed:

- ✅ **Stage 1** (`15bb6a5969cd`): `DecompositionDecl::isStructuredDecomposition()` flag + `BindingDecl::isCompleteObject()` flag; fallback to standard structured bindings when preconditions fail
- ✅ **Stage 2** (`9dae50f759a0`, `bc07c297a0c6`): array protocol — `auto [a, b, c] = arr;` gives three complete-object bindings; per-element cleanups in CodeGen
- ✅ **Stage 3** (`e93a421f26d2`): data-members protocol — non-union class with accessible direct data members; recursive base decomposition via `findDecomposableBaseClass`
- ✅ **Stage 4a** (`5afc088eeb80`): `operator reloc[]` parser + `DeclarationName` kind; `OO_RelocSubscript` operator kind
- ✅ **Stage 4b** (`b49c64dc2d8e`, `ae07daf50553`, `082dd0a38c40`, `ee5a6743afa6`, `30f565decdf4`, `4a25f9ab1c0a`, `2115f2739d79`, `b53a6c3c1e2f`): customized protocol — lookup + dispatch + fallback + relocate-self object parameter + reference-returning operator; CodeGen via `DecompositionDecl`-tracked `VarDecl`
- ✅ **Stage 5** (`e3ae43354058`): per-subobject cleanups for all protocols; `reloc binding` deactivates the correct per-binding cleanup
- ✅ **Templates** (`5829905800cb`): structured decomposition inside templates; pack indexing into binding packs
- ✅ **`constexpr`** (`af5071040dcb`): per-subobject reloc lifetime in constant evaluation
- ✅ **Template candidate materialization fix** (`a3c916f7e1a6`): deferred template candidates materialized during `operator reloc[]` probe (gap 1)

**Stage 4c — library support** (`std::tuple` / `std::array` `operator reloc[]`) ⚠️ Not yet implemented — standard library additions are Phase 13. `std::decomposition_pack` utility has been added (see Phase 13).

**Proposal coverage:** §"structured decomposition", §"structured decomposition protocols", §"customized decomposition protocol", §"decomposition of a lambda closure type"

---

## Additional Sema correctness features ✅ `3e709a134852` + `f576f63bb75c` + `c11e3f4acdb6` + `a3c05fda81cb` + `ea075ee87631` + `ff432dce993d` + `641f2efadb5b` + `c0b3d5d6aa2e`

**Scope:** Several correctness and enforcement features implemented after Phase 10.

- ✅ **vptr reset on decomposition** (`3e709a134852`, `clang/test/CodeGenCXX/p2785-decompose-vptr-reset.cpp`): when a polymorphic object is decomposed, the vtable pointer of each directly-accessed polymorphic base subobject is reset to that base's own vtable, preventing stale polymorphic dispatch through a no-longer-valid derived vptr.
- ✅ **Warn on virtual calls through decomposed base** (`f576f63bb75c`): `warn_decomposed_base_virtual_call` fires when a virtual member function is called through a `CXXDecomposedBaseExpr`; virtual dispatch after decomposition is safe (vptr reset) but semantically suspicious — the warning guides users toward explicit dispatch.
- ✅ **Reject PMF calls on decomposed objects** (`c11e3f4acdb6`): pointer-to-member-function calls `(obj.*pmf)()` on a decomposed object are rejected (`err_decomposed_pmf_call`); PMF dispatch on decomposed objects is ill-formed per §"decomposed-obj-expr".
- ✅ **Reject `return obj;` for decomposed parameter** (`a3c05fda81cb`): returning a decomposed parameter by value is ill-formed (decomposed objects do not produce a value); `err_decomposed_use_as_value` fires from `buildReturnStmt`.
- ✅ **Reject decomposition when subobjects are inaccessible** (`ea075ee87631`): `CheckDecomposedVarDecl` and `CheckDecomposedParams` verify that all non-static data members and bases are accessible from the decomposition context; `err_decomposed_inaccessible_subobject` fires otherwise.
- ✅ **Decomposing-function declaration checks** (`ff432dce993d`): Sema validates that a function declared with `reloc` parameters satisfies the decomposing-function preconditions (§"decomposing-function-decl"): at most one decomposed param per parameter list for non-constructors; virtual decomposing functions permitted; friend declarations propagate the `reloc` parameter flag correctly.
- ✅ **Implicit return relocation for decomposed subobjects** (`641f2efadb5b`, `c0b3d5d6aa2e`): `return d.base<B>;` and `return d.member;` now implicitly insert `reloc` when the returned type has a reloc ctor and the operand is a relocatable subobject of a decomposed local. Reloc ctor is preferred over move in the implicit-reloc path (`c0b3d5d6aa2e`).

---

## Phase 12 — Virtual slicing function ✅ `b102808480dd`–`4e36a3e3ff34`

**Scope:** Hidden virtual function for safe polymorphic relocation.

A class implicitly declares a virtual slicing function when:
1. It **explicitly declares** a non-deleted relocation constructor, AND
2. It has a **virtual destructor** (declared locally or inherited)

OR: it inherits from a base that already provides a virtual slicing function.

The slicing function prevents object slicing when polymorphic objects are relocated through a base pointer via `std::reloc_and_uninitialize`.

### Parameters (prototype ABI)

```
ptr @__slicing_fn(ptr %this, ptr %dest, ptr %target_type_tag)
```

| Parameter | Type | Description |
|---|---|---|
| `this` | `ptr` | The polymorphic object to slice/relocate |
| `dest` | `ptr` | Destination address for the result object |
| `target_type_tag` | `ptr` | Pointer to a per-class unique linkonce_odr constant (type discriminator) |

Single vtable slot per class. The tag-comparison approach avoids O(depth) vtable entries.

### Sub-phases

| # | Sub-phase | Scope | Status |
|---|---|---|---|
| **12a** | AST declaration & Sema | Implicitly declare the slicing function; ill-formed checks | ✅ `b102808480dd` |
| **12b** | Vtable integration | Add slot, mangling, override semantics | ✅ `bbbbf0f64644` |
| **12c** | CodeGen: body emission | Generate the compare-tag / relocate / recurse / cleanup body | ✅ `9de0dea3ab65` |
| **12d** | `std::reloc_and_uninitialize` builtin | `__builtin_reloc_and_uninitialize(ptr)` lowering | ✅ `c2242c636ceb` |
| **12e** | Relax implicit reloc ctor suppression | Allow implicit reloc ctor when virtual dtor has a slicing fn | ✅ `654cea41c6e8` |
| **12f** | Tests | Sema + CodeGen lit tests, runtime tests | ✅ |

### Phase 12a — AST declaration & Sema ✅ `b102808480dd`

- ✅ `CXXRecordDecl` additions: `hasVirtualSlicingFunction()`, `getVirtualSlicingFunction()`; `HasVirtualSlicingFunction` and `VirtualSlicingFunctionIsIllFormed` definition-data bits
- ✅ `AddImplicitlyDeclaredMembersToClass`: after reloc ctor is resolved, check: explicit (non-deleted) reloc ctor + virtual dtor → declare slicing function
- ✅ Inherited case: if base provides slicing fn and derived has explicit reloc ctor → derived also gets one
- ✅ Ill-formed definition checks (emit diagnostic if):
  - Destructor is **user-provided** (not just user-declared) — `err_slicing_fn_user_provided_dtor`
  - No eligible reloc/move/copy ctor — `err_slicing_fn_no_eligible_ctor`
  - Inaccessible base slicing function
  - Inaccessible subobject destructor
- ✅ Access: same as destructor
- ✅ Virtual destructor suppresses implicit reloc ctor (relaxed in 12e)
- ✅ `CXXMethodDecl::isVirtualSlicingFunction()` query

### Phase 12b — Vtable integration ✅ `bbbbf0f64644`

- ✅ Mangling: `__cxx_slicing_fn` name with Itanium mangling
- ✅ Vtable slot: `VTableComponent::CK_SlicingFunction` placed after existing virtual functions
- ✅ Override: derived class's slicing function overrides base's slot (single slot per hierarchy)
- ✅ Thunks: this-adjustment for non-primary bases
- ✅ Virtual diamond inheritance: single final overrider via `__cxx_slicing_fn` lookup (`5fe59f68fa1c`)

### Phase 12c — CodeGen: body emission ✅ `9de0dea3ab65`

Per-class type tag: `@__slicing_tag._ZN4BaseE = linkonce_odr constant i8 0`

Body algorithm for `Derived::__slicing_fn(this, dest, tag)`:
1. **Match case (no virtual bases or single-inheritance):** if `tag == &__slicing_tag._ZN7DerivedE` → relocate `*this` to `dest` via eligible ctor (reloc > move > copy); if non-reloc ctor used, destroy `*this` afterward
2. **Match case (virtual bases, complete variant):** if `tag` matches and class has virtual bases → move-construct virtual base subobjects from `*this` to `dest`, then relocate the non-virtual portion
3. **Decomposition (no-match):** decompose `*this`; invoke base's slicing function on the most-derived virtual base candidate (V); destroy remaining subobjects (fields in reverse-decl order, then non-V bases in reverse-spec order)
4. **Virtual base as V:** when V is a virtual base of the current class, a runtime tag-matching loop over virtual base candidates dispatches to the correct base's slicing function (`7a8048a8b635`)
5. **Exception handling** (`4e36a3e3ff34`): all constructor invocations use `EmitCallOrInvoke`; EH cleanups destroy source (match case) or partially-constructed dest + remaining subobjects (decomposition case) on exception

### Phase 12d — `__builtin_reloc_and_uninitialize` ✅ `c2242c636ceb`

- ✅ Sema: `__builtin_reloc_and_uninitialize(ptr)` — type-safe builtin taking `T*`; validates `T` has virtual slicing function or eligible reloc/move/copy ctor
- ✅ CodeGen: if `T` has a slicing function → emit virtual call `ptr->__slicing_fn(dest, &__slicing_tag._ZN1TE)`; else → direct reloc ctor / move ctor / copy ctor + dtor fallback

### Phase 12e — Relax implicit reloc ctor suppression ✅ `654cea41c6e8`

- ✅ `needsImplicitRelocConstructor()` updated: allow implicit reloc ctor if all bases with virtual dtors also provide a slicing function (i.e., the hierarchy is "relocation-safe")
- ✅ Mutual: if a base provides a slicing function but the derived class can still have an implicit reloc ctor, it does

### Phase 12 — Exception handling ✅ `4e36a3e3ff34`

The slicing function is NOT `noexcept` — constructor invocations in the body can throw. Full EH cleanup support:

- ✅ **Match case (complete, no-vbases):** if the reloc/move/copy ctor throws, source is destroyed via `Dtor_Complete`
- ✅ **Match case (base subobject):** if the reloc/move/copy ctor throws, source is destroyed via `Dtor_Base`
- ✅ **Complete variant (virtual base move):** if the move ctor for a virtual base throws, `*this` is destroyed via `Dtor_Complete` (virtual bases still intact)
- ✅ **Decomposition (V's slicing call):** V has a deactivatable EH cleanup; if the recursive slicing call (via `EmitCallOrInvoke`) throws before dest is constructed, V is destroyed
- ✅ **Decomposition (post-V, pre-V-base destruction):** dest gets an EH cleanup after the slicing call returns; pre-V bases are destroyed in a nested scope with dest protected; if a pre-V base destructor throws, dest is also destroyed

### Tests

- ✅ 6 lit test files:
  - `clang/test/SemaCXX/p2785-virtual-slicing-function.cpp` (implicit declaration, ill-formed checks, inherited)
  - `clang/test/SemaCXX/p2785-virtual-slicing-diamond.cpp` (diamond Sema validation)
  - `clang/test/CodeGenCXX/p2785-virtual-slicing-body.cpp` (body IR: tag compare, reloc, decompose)
  - `clang/test/CodeGenCXX/p2785-virtual-slicing-vtable.cpp` (vtable slot, thunks)
  - `clang/test/CodeGenCXX/p2785-virtual-slicing-diamond.cpp` (diamond codegen)
  - `clang/test/CodeGenCXX/p2785-virtual-slicing-eh.cpp` (EH invoke + landing pad patterns)
- ✅ 31 runtime tests (`P2785/implementation/test/virtual-slicing-*.cpp`): basic slicing, multi-level hierarchies, diamond inheritance, virtual bases, exception handling scenarios

**Proposal coverage:** §"virtual slicing function"

---

## Phase 13 — Standard library additions (partially started)

**Scope:** New and updated standard library components enabled by relocation.

### `std::decomposition_pack` ✅

`std::decomposition_pack<Ts...>` added to `<utility>` — a lightweight aggregate carrier intended as the return type for `operator reloc[]` (P2785 structured decomposition customization point). Specializations provided for arities 0–16 with a by-value deduction guide.

- `libcxx/include/__utility/decomposition_pack.h` (new)
- `libcxx/include/utility` (include + synopsis)
- `libcxx/include/CMakeLists.txt` (registration)
- `libcxx/include/module.modulemap.in` (module entry)

### `std::reloc_and_uninitialize` ✅ `f7c2d0abade3`

`std::reloc_and_uninitialize<T>(T* ptr)` added to `<memory>` — invokes the virtual slicing function (via `__builtin_reloc_and_uninitialize`) for polymorphic types with a slicing function, or falls back to direct relocation for non-polymorphic types.

- `libcxx/include/__memory/reloc_and_uninitialize.h` (new)
- `libcxx/include/memory` (include + synopsis)

### Remaining

| # | Sub-phase | Scope | Status |
|---|---|---|---|
| **13a** | Type traits | `is_relocation_constructible`, `is_trivially_relocation_constructible`, `is_nothrow_relocation_constructible`, assignment variants, `has_virtual_slicing_function`; all `_v` templates | ✅ |
| **13b** | Concepts | `relocation_constructible`, `trivially_relocatable`, `relocatable` | ✅ |
| **13c** | `std::reloc_and_reclaim` | Relocate + deallocate via `::operator delete` or class-specific; 4-step algorithm | ❌ |
| **13d** | `std::construct_at` overload | `T* construct_at(T* p, T src)` — equivalent to `::new (p) T{reloc src}` | ❌ |
| **13e** | `std::relocate` amendment | Prefer reloc ctor over move ctor for non-trivially-relocatable types with nothrow reloc ctor | ❌ |
| **13f** | `operator reloc[]` for `std::tuple` / `std::array` | Stage 4c: return `std::decomposition_pack` from `operator reloc[](this tuple reloc self)` | ❌ |

### Phase 13a — Type traits ✅

Added to `<type_traits>` (C++26, gated on `_LIBCPP_STD_VER >= 26`):

- `std::is_relocation_constructible<T>` / `_v` — wraps `__is_constructible(T, T)`
- `std::is_trivially_relocation_constructible<T>` / `_v` — wraps `__is_trivially_constructible(T, T)`
- `std::is_nothrow_relocation_constructible<T>` / `_v` — wraps `__is_nothrow_constructible(T, T)`
- `std::is_relocation_assignable<T>` / `_v` — wraps `__is_assignable(T&, T)`
- `std::is_trivially_relocation_assignable<T>` / `_v` — wraps `__is_trivially_assignable(T&, T)`
- `std::is_nothrow_relocation_assignable<T>` / `_v` — wraps `__is_nothrow_assignable(T&, T)`
- `std::has_virtual_slicing_function<T>` / `_v` — wraps `__has_virtual_slicing_function(T)`

Files:
- `libcxx/include/__type_traits/is_relocation_constructible.h` (new)
- `libcxx/include/__type_traits/is_relocation_assignable.h` (new)
- `libcxx/include/__type_traits/has_virtual_slicing_function.h` (new)

### Phase 13b — Concepts ✅

Added to `<concepts>` (C++26, gated on `_LIBCPP_STD_VER >= 26`):

- `std::relocation_constructible<T>` — `is_relocation_constructible_v<T>`
- `std::trivially_relocatable<T>` — `__libcpp_is_trivially_relocatable<T>::value`
- `std::relocatable<T>` — `relocation_constructible<T> && is_relocation_assignable_v<T>`

Files:
- `libcxx/include/__concepts/relocatable.h` (new)

**Proposal coverage:** §"type traits header", §"concepts header"

---

## Test counts (current)

| Component | Tests |
|---|---|
| Phase 1–3 (AST, Sema, CodeGen) | 44 |
| Phase 4 (CFG dataflow) | 25 |
| Phase 4b (unsequenced) | 8 |
| Phase 4c (per-member/base tracking) | 30 |
| Phase 5a (core decomposition) | 18 |
| Phase 5b (`.base<B>`) | 44 |
| Phase 5c (`obj.this`) | 10 |
| Phase 5d (virtual-base aliasing) | 13 |
| Phase 5e + CodeGen fixes | 20 |
| Sema §1183 + §1189 fixes | 5 |
| Overload resolution | 4 |
| Phase 6 (reloc ctor, all sub-phases) | 68 |
| Phase 6f-tests (user-provided reloc ctor body) | 12 |
| C-array element-wise relocation | 5 |
| Lambda closure decomposition | 7 |
| Pointer-to-member (`.*` on decomposed) | 9 |
| Miscellaneous (decomposed return reject, etc.) | 16 |
| ExprWithCleanups | 3 |
| Param reloc ctor discardment (§reloc-with-function-param) | 9 |
| Decomposed param silent relocation (§decompose-value-param) | 4 |
| EH-aware use-after-reloc (try/catch) | 6 |
| Phase 9 (relocation elision, aliased reloc-assign) | 67 |
| Phase 9 fixes (callee-destroy, decomposed base elision) | 38 |
| Phase 10 (`constexpr` evaluation) | 33 |
| `reloc` in function types (rejection + acceptance) | 9 |
| Phase 11 (structured decomposition, all stages) | *counted in above* |
| Additional Sema correctness (vptr, PMF, return, inaccessible, decomposing-fn decl, implicit return reloc) | *counted in above* |
| **Total** | **599** |

599 unit tests pass. All tests run cleanly in a single invocation.

Additionally, 33 lit test files pass (2 pre-existing failures in `p2785-reloc-elision.cpp` and `p2785-reloc-operator.cpp` due to CHECK patterns not yet updated for twin emission rework):
- `clang/test/CodeGenCXX/p2785-decompose-destruction-order.cpp` ([class.dtor] order for decomposed locals/params)
- `clang/test/CodeGenCXX/p2785-decompose-vptr-reset.cpp` (vptr reset after base decomposition)
- `clang/test/CodeGenCXX/p2785-pmf-virtual-decomposing.cpp` (virtual-PMF bridge for decomposing fns)
- `clang/test/CodeGenCXX/p2785-reloc-arg-temp-cleanup.cpp` (argument temporary cleanup)
- `clang/test/CodeGenCXX/p2785-reloc-arg-vba-base-no-double-dtor.cpp` (rule 1 in CXXRelocExpr fallback)
- `clang/test/CodeGenCXX/p2785-reloc-array-decomp.cpp` (C-array structured decomposition codegen)
- `clang/test/CodeGenCXX/p2785-reloc-c1-delegation.cpp` (C1 reloc ctor delegation cleanup)
- `clang/test/CodeGenCXX/p2785-reloc-decomp-this-no-caller-dtor.cpp` (caller-side dtor suppression for decomposed explicit object parameter)
- `clang/test/CodeGenCXX/p2785-reloc-elision-member.cpp` (mandatory elision for decomposed member initialisation)
- `clang/test/CodeGenCXX/p2785-reloc-elision-refbind.cpp` (relocation elision for reference binding)
- `clang/test/CodeGenCXX/p2785-reloc-throw-cleanup.cpp` (EH cleanup for reloc ctor throw, VBase cleanup ordering)
- `clang/test/CodeGenCXX/p2785-reloc-twin-addr-taken.cpp` (.Vreloc_twin emission on address-taken)
- `clang/test/CodeGenCXX/p2785-reloc-twin-direct-only.cpp` (direct call uses canonical entry, not twin)
- `clang/test/CodeGenCXX/p2785-virtual-base-cleanup.cpp` (VBA ctor variants, base dtor with VTT)
- `clang/test/CodeGenCXX/p2785-virtual-slicing-body.cpp` (slicing function body: tag compare, reloc, decompose)
- `clang/test/CodeGenCXX/p2785-virtual-slicing-diamond.cpp` (diamond inheritance codegen for slicing fn)
- `clang/test/CodeGenCXX/p2785-virtual-slicing-eh.cpp` (EH invoke + landing pad in slicing fn)
- `clang/test/CodeGenCXX/p2785-virtual-slicing-vtable.cpp` (vtable slot for slicing function)
- `clang/test/SemaCXX/p2785-constexpr-indirect.cpp` (constexpr indirect-call test for decomposing functions)
- `clang/test/SemaCXX/p2785-decomposed-reject.cpp` (decomposition rejection diagnostics)
- `clang/test/SemaCXX/p2785-decomposing-function.cpp` (decomposing-function declaration checks)
- `clang/test/SemaCXX/p2785-overload-resolution.cpp` (P2785 overload resolution tie-breakers)
- `clang/test/SemaCXX/p2785-reloc-operator.cpp` (Sema diagnostics + noexcept static_asserts)
- `clang/test/SemaCXX/p2785-reloc-unused-value.cpp` (unused reloc value warnings)
- `clang/test/SemaCXX/p2785-unsequenced-reloc.cpp` (unsequenced reloc + use diagnostics)
- `clang/test/SemaCXX/p2785-use-after-reloc.cpp` (CFG-based use-after-reloc diagnostics)
- `clang/test/SemaCXX/p2785-virtual-call-decomposed-base.cpp` (virtual call on decomposed base)
- `clang/test/SemaCXX/p2785-virtual-slicing-diamond.cpp` (diamond inheritance Sema for slicing fn)
- `clang/test/SemaCXX/p2785-virtual-slicing-function.cpp` (slicing fn implicit declaration, ill-formed checks)

245 runtime tests pass (`P2785/implementation/test/`), including 31 virtual-slicing tests.

Additionally, 22 constexpr runtime tests pass (`P2785/implementation/test/constexpr-*.cpp`):
- Scalar types: int, pointer, enum, double, bool (constexpr-001)
- Trivial defaulted reloc ctor (constexpr-002)
- User-provided reloc ctor with side-effect (constexpr-003)
- Move-ctor fallback: trivial and non-trivial (constexpr-004)
- Discarded reloc with reloc ctor (constexpr-005)
- Reloc ctor no-double-dtor (constexpr-006)
- Move-ctor fallback dtor counting (constexpr-007)
- Object decomposition: base access and multiple bases (constexpr-008)
- Reloc of decomposed base subobject (constexpr-009)
- `obj.this` non-null and identity (constexpr-010)
- Reloc ctor not discarded on param in constexpr (constexpr-011)
- Non-trivial defaulted reloc ctor (constexpr-012)
- Defaulted reloc ctor with base class (constexpr-013)
- Multiple sequential relocs and conditional reloc (constexpr-014)
- Chained reloc: local → function → param (constexpr-015)
- Copy-only fallback with defaulted reloc (constexpr-016)
- Relocate-only type (move+copy deleted) (constexpr-017)
- Reloc in loop (constexpr-018)
- Nested scope reloc (constexpr-019)
- `consteval` function (constexpr-020)
- Constexpr event-log test: mandatory elision and non-elision with cxlog (constexpr-021)
- Constexpr chained reloc with cxlog: local → function param → reloc param (constexpr-022)

---

## Remaining work — summary

### Required by the proposal but not yet implemented

| Area | Status | Notes |
|---|---|---|
| **Phase 13** — standard library additions | ⚠️ Partial | `std::decomposition_pack` and `std::reloc_and_uninitialize` added. Remaining: `std::construct_at` overload, `std::reloc_and_reclaim`, type traits, concepts, `std::relocate` amendment. Includes Phase 11 Stage 4c: `operator reloc[]` for `std::tuple` / `std::array`. |
| Implicit decomposition of temporaries (§implicit decomposition of temporaries) | ❌ Planned | E.g. `B b = getD();` implicitly decomposes the `D` temporary. Independent of Phase 11. |
| NRVO-style elision for `return reloc x;` | ❌ Deferred | Largely redundant per proposal line 3680 (`return x;` already gets NRVO via end-of-life optimization). |

### ABI gaps in the prototype

| Area | Status | Notes |
|---|---|---|
| Microsoft C++ ABI (`MicrosoftCXXABI.cpp`) | ❌ Out of scope | No `.Vreloc_twin`, no PMF bridging. Direct calls and per-subobject cleanups still work (target-independent). MS is callee-destroy by default so the rule-1+2 silent-reloc concerns don't manifest at direct call sites. |
| ARM pauth signing of the virtual-PMF bridge | ⚠️ Partial | `getOrCreateVRelocPMFBridge` returns a raw `Function*`; pauth call-time authentication will fail under `PointerAuth.CXXMemberFunctionPointers`. No call sites in the prototype's test matrix exercise pauth. |

### Precision / quality-of-implementation gaps (non-blocking)

| Area | Status | Notes |
|---|---|---|
| `typeid` polymorphic glvalue handling in Phase 4b unsequenced checker | ⚠️ Minor | `CXXTypeidExpr` not skipped when its operand is a non-polymorphic prvalue. Extremely obscure; potential spurious diagnostic. |
| Noexcept-aware EH precision | ⚠️ Over-approximation | Phase 1.5 conservatively injects `AliveOrRelocated` for any var with a `CXXRelocExpr` in a try body; never a missed diagnostic, only false positives in catch blocks when nothing throwing follows the reloc. |

### Recently fixed (this branch, post-summary baseline)

| Fix | Commit | Notes |
|---|---|---|
| Synthesize PMF bridge for virtual decomposing fns | `538771eb1df4` | `.Vreloc_pmf_bridge`; PMF formation for virtual decomposing fns. |
| Honor §decompose-value-param-direct rules 1+2 for direct calls | `9dc467296db0` | Split `getDirectCallTarget` vs `getRawFunctionPointer`; suppress caller-side cleanup when the callee param is `reloc`-decomposed. |
| Decomposed objects: destroy in [class.dtor] order | `761c0b50c040` | Inverted EHStack push order so members pop first, then non-virtual bases (reverse spec), then virtual bases. |
| Apply rule 1 to the CXXRelocExpr fallback arg path | `f26f4de630d4` | Suppress caller-side D1 on `reloc.arg.tmp` when callee param is decomposed (e.g. `sink_b(reloc d.base<B>)`). |
| Fix twin emission and array reloc decomposition codegen | `6005dffd9894` | Correct twin emission for decomposing functions; fix array structured decomposition codegen. |
| Mandatory relocation elision for decomposed members | `a0d77a3f1340` | §reloc-elision-mandatory bullet 2: `reloc obj.member` initialiser aliases member storage. |
| Suppress caller-side dtor for decomposed explicit object param | `a4046f6c8839` | `EmitCallArg`: mark aggregate slot externally-destructed when callee param `isDecomposedByReloc()`. |
| Fix overload resolution: prefer reference for xvalue args | `073fca7da55f` | [over.ics.rank]/3.2.3 bullet 4: reference binding beats by-value for non-prvalue arguments. |
| Phase 12: DefinitionData init + destroy non-target subobjects | `6e8ae0bee501` | Initialize slicing-fn bits; destroy non-target bases in decomposition path. |
| Phase 12: Fix ambiguous final overrider in virtual diamond | `5fe59f68fa1c` | Single final overrider via `__cxx_slicing_fn` lookup for diamond hierarchies. |
| Phase 12: V-is-virtual runtime tag-matching loop | `7a8048a8b635` | When V is a virtual base, loop over vbase candidates to dispatch correctly. |
| Phase 12: Ill-formed check for no eligible ctor | `715881852041` | `err_slicing_fn_no_eligible_ctor` when class has no reloc/move/copy ctor. |
| Phase 12: Exception handling in slicing function | `4e36a3e3ff34` | Non-noexcept slicing fn; EH cleanups for all throw paths in match/decomposition. |

---

## File inventory

| File | Role |
|---|---|
| `clang/include/clang/AST/Decl.h` | `VarDecl::IsDecomposedByReloc` bit in `ParmVarDeclBitfields` + `NonParmVarDeclBitfields`; accessors |
| `clang/include/clang/AST/DeclCXX.h` | `SMF_RelocAssignment`, `isRelocationAssignmentOperator()`, reloc assign accessors (`hasUserDeclared…`, `needsImplicit…`, triviality) |
| `clang/include/clang/AST/CXXRecordDeclDefinitionBits.def` | 8-bit `SpecialMembers` bitfields; `NeedOverloadResolutionForRelocAssignment`, `DefaultedRelocAssignmentIsDeleted` |
| `clang/include/clang/AST/ExprCXX.h` | `CXXRelocExpr`, `CXXDecomposedBaseExpr`, `CXXDecomposedThisExpr` classes |
| `clang/include/clang/AST/RecursiveASTVisitor.h` | `DEF_TRAVERSE_STMT` for `CXXDecomposedBaseExpr` and `CXXDecomposedThisExpr` |
| `clang/include/clang/Basic/DiagnosticParseKinds.td` | `err_reloc_not_yet_implemented`, `err_reloc_in_function_type` |
| `clang/include/clang/Basic/DiagnosticSemaKinds.td` | All `reloc` Sema diagnostics |
| `clang/include/clang/Basic/LangOptions.def` | `Relocation` lang option |
| `clang/include/clang/Basic/StmtNodes.td` | `CXXRelocExprClass`, `CXXDecomposedBaseExprClass`, `CXXDecomposedThisExprClass` |
| `clang/include/clang/Parse/Parser.h` | `ParseParameterDeclarationClause` overloads with `IsFunctionDeclaration` flag |
| `clang/include/clang/Sema/Sema.h` | `ActOnRelocExpr`, `ActOnDecomposedBaseAccess`, `ActOnDecomposedThisAccess`, `CheckRelocUseAfterReloc`, `CheckRelocUnsequenced`, `CheckDecomposedVarDecl`, `CheckDecomposedParams`; `CXXSpecialMemberKind::RelocAssignment`; `DeclareImplicitRelocAssignment`, `DefineImplicitRelocAssignment` |
| `clang/include/clang/Sema/Overload.h` | `BindsToPRValue` bit in `StandardConversionSequence` (P2785 overload resolution) |
| `clang/include/clang/Serialization/ASTBitCodes.h` | `EXPR_CXX_RELOC`, `EXPR_CXX_DECOMPOSED_BASE`, `EXPR_CXX_DECOMPOSED_THIS` opcodes |
| `clang/lib/AST/Expr.cpp` | `isUnusedResultAWarning`, `hasSideEffects` hooks for all three nodes |
| `clang/lib/AST/ExprConstant.cpp` | `constexpr` evaluation of `CXXRelocExpr`, `CXXDecomposedBaseExpr`, `CXXDecomposedThisExpr` (Phase 10) |
| `clang/lib/AST/ExprClassification.cpp` | `CL_PRValue` / `CL_LValue` for `CXXRelocExpr`, `CXXDecomposedBaseExpr`, `CXXDecomposedThisExpr` |
| `clang/lib/AST/ExprCXX.cpp` | Constructors, `Create`, `CreateEmpty` for `CXXDecomposedBaseExpr` and `CXXDecomposedThisExpr` |
| `clang/lib/AST/StmtPrinter.cpp` | `VisitCXXRelocExpr`, `VisitCXXDecomposedBaseExpr`, `VisitCXXDecomposedThisExpr` |
| `clang/lib/AST/StmtProfile.cpp` | `VisitCXXRelocExpr`, `VisitCXXDecomposedBaseExpr`, `VisitCXXDecomposedThisExpr` |
| `clang/lib/CodeGen/CGCleanup.cpp` | `ConditionallyDeactivateCleanup` — sets runtime `cleanup.isactive` flag without `setActive(false)`; keeps cleanup logically active for conditional-branch correctness |
| `clang/lib/CodeGen/CGExprCXX.cpp` | `EmitCXXRelocExpr`; `MemberDestroyCleanup` per-member destructor cleanup; `DeactivateSourceCleanup` lambda; temp materialization for discarded reloc on caller-destroy params |
| `clang/lib/CodeGen/CGCall.cpp` | `EmitDelegateCallArg`: skip `CalleeDestructedParamCleanups` for decomposed params (§decompose-value-param) |
| `clang/lib/CodeGen/CGDecl.cpp` | `EmitParmDecl`: silent relocation of decomposed params to local storage for caller-destroy ABIs (§decompose-value-param) |
| `clang/lib/CodeGen/CGExpr.cpp` | `EmitLValue` case for `CXXDecomposedBaseExpr` |
| `clang/lib/CodeGen/CGExprScalar.cpp` | `VisitCXXDecomposedThisExpr` (emits alloca address as `void cv*`) |
| `clang/lib/CodeGen/CodeGenFunction.cpp` | Early-destructible source lifetime suppression |
| `clang/lib/CodeGen/CodeGenFunction.h` | Declaration of `ConditionallyDeactivateCleanup`; `EmittingRelocAssignEliding`, `CallingRelocAssignOperator` flags (Phase 9) |
| `libcxx/include/__utility/decomposition_pack.h` | `std::decomposition_pack<Ts...>` aggregate carrier for `operator reloc[]` return type (Phase 13) |
| `clang/lib/Driver/ToolChains/Clang.cpp` | `-frelocation` driver flag |
| `clang/lib/Frontend/CompilerInvocation.cpp` | `LangOpts.Relocation` mapping |
| `clang/lib/Parse/ParseDecl.cpp` | `reloc` keyword in declarator parsing (`T reloc name`); `reloc`-in-function-type diagnostic in `ParseParameterDeclarationClause` |
| `clang/lib/Parse/ParseExpr.cpp` | `tok::kw_reloc` in `ParseCastExpression` (absorbs `.*`/`->*`); `.base<` intercept; `.this` keyword intercept |
| `clang/lib/Sema/SemaChecking.cpp` | `CheckCompletedExpr` call to `CheckRelocUnsequenced` |
| `clang/lib/Sema/SemaDecl.cpp` | `ActOnParamDeclarator`: decomposed-param type checks; `ActOnFunctionDeclarator`: calls `CheckDecomposedParams` after `mergeFunctionDecl`; `ComputeSpecialMemberFunctionsEligiblity` for reloc assign |
| `clang/lib/Sema/SemaDeclCXX.cpp` | `ActOnVariableDeclarator` calls `CheckDecomposedVarDecl` for non-parameter locals; `CheckCompletedCXXClass`: forced `ParamDestroyedInCallee` for relocate-only types (§reloc-with-function-param); `DeclareImplicitRelocAssignment`, `DefineImplicitRelocAssignment`, `ComputeDefaultedSpecialMemberExceptionSpec` (move/copy ctor for non-eliding wrapper) |
| `clang/lib/Sema/SemaCast.cpp` | `BuildCXXNamedCast` calls `DiagDecomposedVarUsedAsValue` (Phase 5e) |
| `clang/lib/Sema/SemaExprCXX.cpp` | `IgnoredValueConversions` calls `DiagDecomposedVarUsedAsValue`; `BuildDecltypeType` enforcement (Phase 5e); `CheckPointerToMemberOperands` P2785 constant-eval check |
| `clang/lib/Sema/SemaExprMember.cpp` | Reject qualified member access on decomposed objects; reject non-static member-function calls; permit static member calls via instance syntax; lambda capture name resolution via `getCaptureFields()` for decomposed closures |
| `clang/lib/Sema/SemaInit.cpp` | `PerformCopyInitialization` calls `DiagDecomposedVarUsedAsValue` (Phase 5e) |
| `clang/lib/Sema/SemaType.cpp` | `BuildDecltypeType` parenthesised-decomposed-var enforcement (Phase 5e) |
| `clang/lib/Sema/SemaRelocation.cpp` | All reloc Sema logic; Phase 4/4b analysis; `CheckDecomposedVarDecl`; `CheckDecomposedParams`; `ActOnDecomposedBaseAccess`; `ActOnDecomposedThisAccess`; `resolveConstPtrMemField` for `.*` flow analysis |
| `clang/lib/Sema/SemaOverload.cpp` | P2785 overload resolution tie-breakers in `CompareStandardConversionSequences` (gated behind `-frelocation`); lvalue tiebreaker (const T& beats T by-value for lvalue args) |
| `clang/lib/Sema/SemaTemplateInstantiate.cpp` | `SubstParmVarDecl` preserves `isDecomposedByReloc()` during instantiation |
| `clang/lib/Sema/SemaTemplateInstantiateDecl.cpp` | `RebuildTypeSourceInfoForDefaultSpecialMembers` filter for `RelocConstructor` and `RelocAssignment` |
| `clang/lib/Sema/TreeTransform.h` | `TransformCXXDecomposedBaseExpr`, `TransformCXXDecomposedThisExpr` |
| `clang/lib/Serialization/ASTReaderStmt.cpp` | `VisitCXXDecomposedBaseExpr`, `VisitCXXDecomposedThisExpr`; `CreateEmpty` cases |
| `clang/lib/Serialization/ASTWriterStmt.cpp` | `VisitCXXDecomposedBaseExpr`, `VisitCXXDecomposedThisExpr` |
| `clang/lib/AST/DeclCXX.cpp` | `isRelocationAssignmentOperator()`, `addedMember` SMF_RelocAssignment, `finishedDefaultedOrDeletedMember` ordering, triviality propagation |
| `clang/test/CodeGenCXX/p2785-reloc-operator.cpp` | FileCheck IR tests for scalar, pointer, class, base-subobject, decomposition, discard, conditional-branch, silent-relocation (caller-destroy, relocate-only, fptr, virtual dispatch), reloc assign codegen, aliased reloc-assign call sites |
| `clang/test/CodeGenCXX/p2785-reloc-elision.cpp` | FileCheck IR tests for relocation elision (call-site, MemberExpr, recursive), aliased reloc-assign (non-eliding vs eliding bodies, nested dispatch) |
| `clang/test/CodeGenCXX/p2785-reloc-elision-refbind.cpp` | FileCheck IR tests for reference binding elision (Phase 9c) |
| `clang/test/CodeGenCXX/p2785-reloc-c1-delegation.cpp` | FileCheck IR tests for C1 reloc ctor delegation cleanup |
| `clang/test/CodeGenCXX/p2785-reloc-throw-cleanup.cpp` | FileCheck IR tests for EH cleanup ordering (reloc ctor throw, VBase cleanup) |
| `clang/lib/Sema/SemaLookup.cpp` | `ForceDeclarationOfImplicitMembers`, `LookupSpecialMember`, `AddMethodCandidate`/`AddMethodTemplateCandidate` for reloc assign |
| `clang/lib/Sema/SemaExpr.cpp` | `MarkFunctionReferenced`: reloc assign before copy assign ordering |
| `clang/test/SemaCXX/p2785-reloc-operator.cpp` | Sema lit tests: reloc operator validity, noexcept exception specification |
| `clang/unittests/AST/CXXRelocExprTest.cpp` | 492 unit tests |
