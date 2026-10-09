# Mu IR — model and text format

**Status:** draft for review. Describes *the way the IR is supposed to be*,
derived from `wallvm` (`irtypes`, `irOps`, builder). Text names for opcodes,
predicates, and orderings are exactly the strings `irOpcodeStr()` /
`irICmpStr()` / `irFCmpStr()` / `irAtomicOrderingStr()` already emit.

---

## 1. Mental model

- A **graph of typed values**. Everything that produces data — instructions,
  constants, arguments, blocks, functions, globals, strings — is an
  `IrValue` with a type, a unique id, and an explicit **user list**.
- A **function is a list of basic blocks**; a block is a list of
  instructions; the last instruction is the terminator. CFG edges are stored
  on the blocks themselves (`preds`/`succs`) in addition to being implied by
  terminators.
- **Not strict SSA at construction.** Locals are `alloca` + `load`/`store`;
  `phi` exists for direct SSA emission or after a promotion pass. The front
  end may choose either style; the graph shape, not the construction order,
  is what the IR guarantees.
- **All shape information is on the instruction**: result type, operand
  list, compare predicate, atomic ordering. Nothing requires looking up a
  type to know what an instruction *is*.
- **Constants are uniqued** in the context: the same integer/float constant
  is the same pointer, so operand identity is pointer identity.
- Debug location, per-variable debug info, and key/value **metadata** attach
  to instructions, functions, and globals (§7.4).

## 2. Object graph

```
IrContext            — shared arena/uniquer: types, const pools, value ids,
│                      analysis caches
└── IrModule          — named compilation unit
    ├── IrFunc        — name, signature, extern lib/sym, blocks, metadata
    │   └── IrBlock   — name (value), insts, preds, succs
    │       └── IrInst— opcode, operands, result type, pred/ordering,
    │                  debug loc, var debug info, metadata, prev/next
    ├── IrGlobal      — name, type, const?, extern?, init, align, metadata
    ├── IrStringConst — name, bytes, length (type = array of i8)
    └── IrStructType  — name, field types/names, computed size + offsets
```

Ownership: the context owns types and constant pools (shared, interned); the
module owns funcs/globals/strings/struct-defs; a func owns its blocks; a
block owns its instructions; `IrValue.users` are back-pointers maintained on
every operand write (`addUser` at emission, `replaceAllUses` on rewriting).

## 3. Types

| Form | Text | Notes |
|---|---|---|
| void | `void` | |
| integer | `i1 i8 i16 i32 i64` | `i1` = bool/condition |
| float | `f32 f64` | |
| pointer | `ptr T` / bare `ptr` | bare `ptr` = raw pointer (≡ ptr to void); `ptr T` = typed |
| array | `array T n` | inline `n × T` |
| vector | `vector T n` | SIMD |
| function | `func T (T, …)` | varargs: `func i32 (ptr, ...)` |
| struct | `struct @Name` / `struct { T, … }` | named (declared at module level) or anonymous |

- Types are **interned**: identical structure ⇒ identical pointer.
- The only peek in type parsing: after `ptr`, if the next token is a type,
  it is the element type; otherwise the pointer is raw. One token, no
  backtracking, and only inside a type — everywhere else types sit at
  fixed positions.
- `usize`/`isize` lower to `i64` on x86_64 (target word size); `bool` is
  `i1`; Mu `char` is `i32` (Unicode scalar).
- `Alloca T` has type `ptr T`; `Load T` yields `T`; `icmp`/`fcmp` yield `i1`.

## 4. Values, constants, names

Kinds: `Inst`, `ConstInt`, `ConstFloat`, `Block`, `Func`, `Global`, `Arg`,
`StringConst`.

- Operands are always **atoms**: a literal, `%name`, `@name`, or (in
  block-operand slots only) a bare block name. The IR has **no infix
  expressions** — `add i32 %a, %b`, never `%a + %b`.
- Text names: named values print their name; unnamed values print `%<id>`
  (the value's unique id) — stable, scope-free, round-trippable.
- Int constants: `42`, `-1`, `0xFF`; `i1` accepts `true`/`false`.
- Float constants print shortest round-trip decimal; raw bit form
  `0x3FF0000000000000` is also accepted (the type position decides whether
  `0x…` is an integer value or float bits).
- String constants are declared once at module level and decay to bare
  `ptr` where an operand is expected.

## 5. Invariants — the way it is supposed to be

1. **Exactly one terminator** per block: `br`, `cond_br`, `ret`,
   `unreachable`, `noret`.
2. **CFG edges match terminators**: if `br`/`cond_br` names a block, that
   block lists the emitter in `preds` and the emitter lists it in `succs`.
   Maintained at emission, never by hand.
3. **Insertion position is honored**: `pos(block, before)` — emit before
   `before`, append when it is `null`. Every helper goes through it.
4. **Use lists stay exact**: every operand write registers the user;
   rewriting a value (`replaceAllUses`) moves every user to the new value
   and leaves no stale entries, including self-references.
5. **`gep` indexes through aggregates**: array index ⇒ element type, struct
   index ⇒ field type (constant index). Result type is `ptr` to the reached
   type. Field offsets come from the `IrStructType` layout (align-aware).
6. **Predicates live on the instruction**: `icmp`/`fcmp` carry their
   predicate; atomics carry ordering (and `atomic_rmw` its operation). No
   side tables needed to print or verify an instruction.
7. **Externs are signature-only**: a `func` with an `extern "lib" "sym"`
   linkage clause has no body, ever.
8. **Metadata is exact**: keys map to an int, string, IR value, or a small
   atom list (§7.4); debug loc and var-debug are ordinary keys
   (`debug.file`/`debug.line`/`debug.col`, `var`). Anything unknown is
   `Custom` and round-trips verbatim.
9. **Types are context-interned; constants are context-uniqued** — struct
   identity for the type system is layout (size + field types); the
   `IrStructType` side-table (names, offsets) rides alongside in the module.

## 6. Opcode catalogue

Operand shapes below are the *text* shapes (§7); `T` = type, `v` = atom,
`L` = block name, `i` = constant integer index.

**Terminators** (no result)

| opcode | shape |
|---|---|
| `br` | `L` |
| `cond_br` | `v, L, L` |
| `ret` | *(void)* or `T v` |
| `unreachable`, `noret` | *(empty)* |

**Integer arithmetic/bitwise** (result: `T`)

| group | opcodes | shape |
|---|---|---|
| binary | `add sub mul sdiv udiv srem urem and or xor shl ashr lshr rotl rotr smin smax umin umax` | `T v, v` |
| unary | `neg not abs ctpop ctlz cttz bswap` | `T v` |

**Floating point** (result: `T`)

| group | opcodes | shape |
|---|---|---|
| binary | `fadd fsub fmul fdiv` | `T v, v` |
| ternary | `fma` | `T a, b, c` |
| unary | `fabs fsqrt` | `T v` |

**Casts** (shape `T v to T2`, result `T2`):
`trunc zext sext fptrunc fpext fptosi sitofp fptoui uitofp bitcast`

**Compare** (result `i1`): `icmp <pred> T v, v` / `fcmp <pred> T v, v`
— preds: `eq ne slt sle sgt sge ult ule ugt uge` /
`oeq one olt ole ogt oge ueq une ult ule ugt uge`

**Memory**

| opcode | shape | result |
|---|---|---|
| `alloca` | `T` | `ptr T` |
| `load` | `T v` | `T` |
| `store` | `T v, ptr-expr` | none |
| `gep` | `v, i…` (indices: constants or atoms) | `ptr` to reached type |

**Control/data**

| opcode | shape | result |
|---|---|---|
| `call` | `T callee(T v, …)` — callee is `@sym` or `%fp` | `T` (`void` ⇒ no result) |
| `phi` | `T [ v, L ], …` | `T` |
| `select` | `i1 c, T a, T b` | `T` |
| `extractvalue` | `T v, i` | field type |
| `insertvalue` | `T agg, T2 v, i` | `T` |

**SIMD** (result `T` unless noted): `vadd vsub vmul vdiv vfadd vfsub
vfmul vfdiv vmin vmax` → `T v, v`; `vbroadcast` → `T v`;
`shufflevector` → `T v1, v2, i…`; `extractelement` → element type;
`insertelement` → `T v, T2 e, i`

**Atomics**

| opcode | shape | result |
|---|---|---|
| `atomic_load` | `T ptr, <ord>` | `T` |
| `atomic_store` | `T v, ptr, <ord>` | none |
| `atomic_rmw` | `<rmwop> T ptr, v, <ord>` | `T` |
| `atomic_cmpxchg` | `T ptr, cmp, v, <ord>, <ord>` | `T` |
| `fence` | `<ord>` | none |

orderings: `relaxed acquire release acq_rel seq_cst`; rmwops:
`xchg add sub and nand or xor max min umax umin`

**Misc**: `gc_root v`, `gc_writebarrier ptr, v`, `gc_safepoint` (none);
`landingpad` reserved (Mu has no catch in v1).

**Inline assembly** (the three strings map 1:1 to `IrInst`'s asm fields):

```
inlineasm [volatile] T "template" : "constraints" : "clobbers" (T v, …)
```

- result `T` (`void` ⇒ no result, so no `%x =`).
- `"template"` — the asm text; operands are referenced positionally (`%0`,
  `%1`, … as in GCC/Clang).
- `"constraints"` — comma-separated, **one entry per operand**, in order
  (`"=a"` output, `"0"` tied input, `"D"` input, `"+r"` read-write, …).
- `"clobbers"` — comma-separated register/resource names
  (`"rcx,r8,r9,r10,r11,memory"`).
- `volatile` — the instruction has side effects (`asmNumSideEffects`);
  omit only for pure computations.
- Operand list `(...)` is the values the template may touch, typed like
  call arguments.

Example:

```
%4 = inlineasm volatile i64 "syscall" : "=a,0,3,4,5,6,7" : "rcx,r11,memory" (%1, %2, %3, %5, %6, %7)
    inlineasm volatile void "mfence" : "" : "memory" ()
```

## 7. Textual format

### 7.1 Shape of a file

```
module "demo"

type @Vec2 = struct { x: f32, y: f32 };
global const @LIMIT: i32 = 10;
global @counter: i32 = 0;
string @s0: "hello";
func @puts(ptr) i32 extern "libc" "puts";

func @add(%a: i32, %b: i32) i32 {
  entry:
    %1 = add i32 %a, %b
    ret i32 %1
}
```

- `module "name"` is a header line (no `;`). Every other top-level decl
  ends with `;`; a function with a body ends with `}`.
- A function's body contains **labels** (`ident:`) and **instructions**
  (one per line). Unnamed results are `%1`, `%2`, … (the value's id);
  named results/params use any identifier.
- Extern (bodyless) form: `func @name(params) [ret] extern "lib" "symbol";`
  — the two strings are the library and the exported symbol (omit the
  second only when it equals `@name`).

### 7.2 Lexical rules

- **Sigils decide namespaces by first character**: `@` symbol
  (func/global/string), `%` value (result, param, argument). Blocks are
  bare names — a line inside a body that reads `ident :` is a label (no
  opcode is ever followed by `:`). These tokens run to the next whitespace
  or `( ) , [ ]`.
- Comments: `//` to end of line.
- **One instruction per line.** Multi-line is allowed only inside `( )` /
  `[ ]` / param lists, where the parser just counts depth.
- Strings: `"…"` with `\n \t \\ \" \xNN` escapes.

### 7.3 Grammar (EBNF)

```
program   := ["module" string] { decl }
decl      := "type"   "@" ident "=" structdef ";"
           | "global" ["const"] "@" ident ":" type ["=" init] [meta] ";"
           | "string" "@" ident ":" string [meta] ";"
           | funcdecl
funcdecl  := "func" "@" ident "(" [params] ")" [type] [linkage] [meta]
             body
linkage   := "extern" string string             // lib, symbol; implies no body
body      := "{" { block } "}"                  // with linkage: never a body
           | ";"                                // linkage present: ends decl
params    := param { "," param } ["," "..."]    // param := ["%" ident ":"] type
structdef := "struct" "{" [field { "," field }] "}"  // field := ident ":" type
block     := ident ":" { instr }                // next ident ":" or "}" ends it
instr     := ["%" ident "="] op [meta]          // one line
type      := "void" | "i1".."i64" | "f32" | "f64"
           | "ptr" [type]                       // type follows ⇒ typed, else raw
           | "array" type int | "vector" type int
           | "func" type "(" [type { "," type }] ")"
           | "struct" ("@" ident | "{" [type { "," type }] "}")
atom      := int | float | "true" | "false" | "%" ident | "@" ident
init      := atom | "[" [init { "," init }] "]" // list elements take the
                                                // element type (array/vector)
op        := <fixed shape per opcode, table in §6>
meta      := "!" ident ["=" mvalue { "," mvalue }]   // §7.4
mvalue    := int | float | "true" | "false" | string
           | "%" ident | "@" ident | type
```

Dispatch after the opcode keyword is a **fixed shape** (§6 table): read
types where the shape says types, atoms where it says atoms, block names
where it says `L`, `to`/`[`/`(` where it says so.

### 7.4 Metadata

Any instruction, function signature, `global`, or `string` may carry
trailing `!key …` clauses, parsed to end of line (or to `;`/`{` in
signatures). A clause is a bare flag or `key = value[, value…]`:

```
%2 = load i32 %1 !alignment = 4 !noalias !debug.file = "demo.mu" !debug.line = 7
func @clamp(%v: i32, %lo: i32, %hi: i32) i32 !inlinehint = 1 !noalias {
```

| key | value | typical attach point |
|---|---|---|
| `debug.file` | string | instruction, function |
| `debug.line` / `debug.col` | int | instruction, function |
| `var` | `"name", T, line, col [, argIndex]` — repeatable, one per tracked local | instruction |
| `alignment` | int | instruction, global |
| `noalias` `nocapture` `nofree` `noreturn` `readnone` `readonly` `writeonly` | bare flag (≡ `= 1`) | instruction, function |
| `inlinehint` | int | function |
| `weight.true` / `weight.false` | int | branch instruction |
| `range.min` / `range.max` | int | value-producing instruction |
| *any other* | any atom list — carried verbatim as `Custom` | anywhere |

Values are single atoms (`7`, `"f.mu"`, `%v`, `i32`) or comma-separated
atom lists (`var`). A bare `!flag` means `= 1`. Debug location and
per-variable debug info (name, type, line, col, arg index) are ordinary
metadata — no separate syntax.

### 7.5 Why this parses in one pass

1. At depth 0, the **first word** is one of a fixed set (`module type
   global string func`); inside a function body the first token decides:
   `ident :` = label, `%` = result-carrying instruction, keyword = void
   instruction. Nothing else can start a line.
2. **No infix expressions anywhere** — operands are single atoms, so the
   parser never builds or prioritizes expression trees.
3. **Every type starts with a keyword** with a closed form; the single
   peek is `ptr` deciding elem-vs-raw (one token, no backtracking), and
   types only ever appear at fixed positions.
4. **Every opcode has exactly one shape** — no alternatives, no optional
   middle clauses (only optional leading `%x =`, the `volatile` marker on
   `inlineasm`, and trailing `!meta`).
5. **Namespaces are sigils** — a token's role is its first character; a
   block name only ever appears in `br`/`cond_br`/`phi` slots.
6. **Fixed line/decl terminators** — instructions are newline-terminated,
   top-level decls `;`-terminated, bodies `}`-terminated; no statement-
   boundary guessing, no semicolon insertion rules.
7. **No implicit names** — every produced value is bound explicitly
   (`%x =` or a named param); no scope bookkeeping like LLVM's unnumbered
   `%3`.

## 8. Example

```
module "demo"

type @Vec2 = struct { x: f32, y: f32 };
global const @LIMIT: i32 = 10;
global @counter: i32 = 0;
string @greet: "hi\n";
func @puts(ptr) i32 extern "libc" "puts";

func @clamp(%v: i32, %lo: i32, %hi: i32) i32 !inlinehint = 1 {
  entry:
    %1 = icmp slt i32 %v, %lo
    cond_br %1, tooLow, checkHi
  tooLow:
    br done
  checkHi:
    %2 = icmp sgt i32 %v, %hi
    cond_br %2, tooHigh, ok
  tooHigh:
    br done
  ok:
    br done
  done:
    %3 = phi i32 [ %lo, tooLow ], [ %hi, tooHigh ], [ %v, ok ]
    ret i32 %3
}

func @setY(%v: ptr struct @Vec2) {
  entry:
    %1 = gep %v, 1
    store f32 2.5, %1
    ret
}

func @main() i32 {
  entry:
    %1 = alloca i32
    store i32 3, %1
    %2 = load i32 %1 !debug.file = "demo.mu" !debug.line = 7
    %3 = call i32 @clamp(i32 %2, i32 0, i32 9)
    call i32 @puts(ptr @greet)
    inlineasm volatile void "mfence" : "" : "memory" ()
    ret i32 %3
}
```

## 9. Mu → IR lowering map

| Mu construct | IR shape |
|---|---|
| `var`/`let` locals | `alloca` + `load`/`store` (promote later), or direct SSA values |
| `if` / `while` / `for` | blocks + `icmp` + `cond_br`; labels become block names |
| expression-position `if`/`switch` | one `alloca` slot per join, or `phi` after promotion |
| `switch`, guards, ranges | `cond_br` cascade (jump tables later); tag/range compares |
| `variant` | tagged struct `{ i32 tag, payload }`; match = load tag + `gep` payload |
| tuples / structs (values) | struct-typed values; field = `gep` (addr-taken) / `extractvalue` |
| `?` operator | early-`ret` block chains per `Err` prong |
| bounds / overflow trap | `icmp` + `cond_br` to a trap block that `call`s the panic handler then `noret` |
| closures | struct `{ fnptr, envptr }`, indirect `call` |
| `dyn Trait` | fat pointer `{ data, vtable* }`; vtable = `global` struct of func ptrs |
| `async fn` | state-machine: frame struct + `alloca` state + `cond_br` dispatch; frame on the heap via injected allocator calls |
| monomorphized generics | ordinary funcs, mangled `@module::name<i32>` symbols |
| `@extern fn` | `func @name(…) T extern "lib" "sym";` — signature only |

## 10. Reserved / notes

- `landingpad`, `gc_root`/`gc_writebarrier`/`gc_safepoint` exist in the
  opcode set but have no Mu source mapping in v1 (kept for the backend's
  sake).
- `noret` marks paths that never return (traps); `unreachable` marks states
  the front end has proven impossible.
- The text format is the print/parse round-trip format: dumps, IR-level
  tests, and any caching of IR go through it. Printer output must reparse
  to the same graph (ids preserved as printed names).
