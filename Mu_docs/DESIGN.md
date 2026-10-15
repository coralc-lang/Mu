# Mu — Language Design Notes

**Mu (μ)** — named after the Greek letter that symbolizes micro. Scripting
language for a game engine, compiled to native code. Everything below is
**Decided** unless listed under Open items — only the SSA IR spec remains
pending from the author.

---

## 1. Lexical / statement rules — **Decided**

- Statements end with **semicolons**: `var x: i32[3];`, `x = x + 1;`
- Blocks use braces; a block itself is not semicolon-terminated.
- Comments are **C-style**: `//` line, `/* ... */` block (non-nesting).
- **Bindings:** `var` = mutable local, `let` = immutable local (initializer
  required), `const` = module-level constant. `var`/`let` are statements;
  `const` lives at module level (§13). Reassigning a `let` is a compile
  error.
- **`@` marks specials:** loop labels (`@outer:`), foreign declarations
  (`@extern fn ...;`), and attributes (`@component struct ...`, §16) — `@`
  starts no expression, so all three are unambiguous.
- Identifiers: `[A-Za-z_][A-Za-z0-9_]*`, case-sensitive.
- **Naming conventions:** types/traits/variant cases `PascalCase`; functions,
  variables, fields, modules `camelCase`; constants `SCREAMING_SNAKE`.
- **Loop labels** are `@`-marked — a bare `outer:` would materialize out of
  nowhere; `@` starts no expression, so declaration and use are both
  unambiguous:
  `@outer: while ... { break @outer; continue @outer; }`
- **Literal forms:** ints `42`, `0xFF`, `0b1010`, `0o755`, separators
  `1_000_000`; floats `1.5`, `1.5e10` (digit required before `.`); chars
  `'a'` with escapes `\n \t \\ \' \u{...}`; strings `"..."` with the same
  escapes plus `\"`.
- **Literal suffixes** force a type when inference must not decide:
  `42i8`, `42u64`, `0xFFu8`, `1.0f32` — the suffix is spelled exactly like the
  type alias. Unsuffixed literals are polymorphic (§3).
- **`pub`** marks exports.

## 2. Syntax skeleton — **Decided**

```
fn name(a: T, b: T) T { ... }              // return type omitted = void
fn swap(a: i32, b: i32) (i32, i32) { ... } // tuple return = multiple values

struct Name {
    field: T;
    fn method(self, ...) T { ... }          // optional, in-body, dot access
    fn new(...) Name { ... }                // associated fn: no receiver
}

trait Name { ... }

extend Type { ... }              // add methods to an existing type
extend Type: trait Name { ... }  // trait extension / conformance block

variant Name { Case, Case2, Case3, ... } // like a regular enum
variant Name { Case, Case2(T), ... }

const LIMIT: i32 = 10;           // pub const for export
use path::to::module;            // binds `module`: module::symbol
use path::to::module as m;       // alias: m::symbol
use path::{a, nested::{b, c}};   // group and nested imports
@extern fn draw(x: f32, y: f32); // foreign fn (engine doesn't provide)
```

- **Struct literals:** `Name { field: value }` — all fields required unless
  `..base` copies the rest (`Name { x: 1, ..old }`); field shorthand allowed
  (`Name { x }` when a variable `x` is in scope).
- **Control-flow heads are struct-literal-free:** in `if` / `while` / `for`
  heads an expression ends at `{` — `if flag { ... }` conditions on `flag`,
  never a struct literal. Parenthesize to use one there:
  `if (Vec2 { x: 1 } == v) { ... }`. Single pass, no backtracking.
- **Method receivers:** `self` binds directly to the receiver's storage —
  methods mutate in place, no copying on call. A method that stores/returns
  `self` **moves** it out of the caller's variable; later use of that variable
  is a move error (definite assignment).
- **Associated functions** (no `self`) are called on the type: `Vec2.new()`.
- **No overloading within a module/scope** — one name = one function.
  Different modules may define the same names (§11); methods are scoped per
  type.
- **No inheritance, no templates (C++ sense), no macros.** Composition only.
- Extension blocks may target types this script does not own, but are active
  **only in files that `use` their module** (§13).

## 3. Type system — **Decided**

### Numeric — fixed width, short aliases
```
i8 i16 i32 i64        signed
u8 u16 u32 u64        unsigned
isize usize           pointer-sized (indices are usize)
f32 f64               floats
```
`int8_t`-style names rejected as too verbose.

### Math types — engine built-ins — **Decided**
- `Vec2 { x: f32, y: f32 }`, `Vec3`, `Quat`, `Mat4` — engine-provided,
  **always in scope**: never declared, never imported.
- They obey §4 value semantics and overload operators through §10
  (`add`, `mul`, `eq`, …), so gameplay reads naturally:
  `var p = pos + dir * speed * dt;`

### `char`
- Unicode **scalar value** (no surrogate halves), stored as 4 bytes.
- Literals `'a'`, `'\u{1F600}'`.
- `char as u32` yields the scalar; `u32 as char` **traps** if not a valid
  scalar.
- `str`/`string` index as bytes; explicit helpers iterate code points
  yielding `char`.

### Conversions
- **Implicit conversion only when the target type is strictly bigger**
  (widening within the same family: `i8→i32`, `u8→u64`, `f32→f64`).
- Even permitted implicit widening **produces a warning** nudging you to be
  explicit.
- Everything else requires **`as`**: `var y = x as i64;`
- **Literal exception:** literals are polymorphic when they fit —
  `var x: u8 = 255;` silent; `var x: u8 = 256;` error. Suffixes (`255u8`)
  force deliberately.

### String
- `string` — built in, **UTF-8**, immutable; operations return new strings or
  views.
- `str` — string slice (view: ptr + len); substrings/searches return `str`check 
  without copying.
- Built-in method groups **toggled on/off per script via the manifest**
  (§14): scripts don't pay for methods they never use.

### Tuples
- `(i32, f64)` — first-class value type, inline layout, no boxing.
- **Multiple returns** are tuples: `fn swap(...) (i32, i32)`; destructured by
  `var (a, b) = swap(1, 2);` and in `for` bindings: `for (k, v) in map { }`.
- `variant` multi-field payloads are tuple-shaped positionally:
  `Rect(w, h)` matches by position.

### Collections — built into the language
- `i32[3]` — **fixed-size array**, inline storage, value type.
- `i32[]` — **slice**: non-owning view (ptr + len); borrow rule in §5.
- `Array<T>` — **owning dynamic array** (growable), built-in generic struct.
- `Map<K, V>` — **hash map**, built-in generic. Keys need built-in support
  (primitives, `char`, `bool`, `string`, tuples/aggregates of hashable fields)
  or `fn hash(self) -> u64` + `fn eq` methods.
- `Set<T>` — **unique membership**, same cost profile as `Map<K, V>` but
  stores values only: `Set.new()`, `.len()`, `.has(v)`, `.add(v)`,
  `.remove(v)`; `for v in set { }` visits the members.
- `Queue<T>` — **FIFO**, for command queues, event queues, and BFS:
  `Queue.new()`, `.push(v)` (enqueue), `.pop()` → `Option<T>` (dequeue,
  `None` when empty), `.len()`; `for v in queue { }` visits front → back.
- `Pool<T>` — **fixed-capacity slot pool**. Created once with `Pool.new(cap)`
  and never grows, so per-frame gameplay code allocates nothing:
  `.create()` → `Option<u32>` hands out a handle (`None` at capacity),
  `.release(h)`, `.get(h)`, `.set(h, v)`, `.alive(h)`, `.len()`, `.cap()`.
  Handles are recycled. Intended for particles, bullets, and pooled
  entities — the no-GC counterpart of "just `new` one".
- `Bitset` — **non-generic bit mask**, sized at run time (`Bitset.new(128)`),
  for collision layers, input masks, and feature flags: `.test(i)`,
  `.set(i)`, `.clear(i)`, `.toggle(i)`, `.any()`, `.all()`, `.none()`,
  `.count()`. It takes **no** type argument — `Bitset<i32>` is E0366.
- All compile to direct, allocation-free code where possible.

### Ranges
- `a..b` (exclusive), `a..=b` (inclusive) construct
  `Range<T> { start, end, inclusive }` — a **first-class value type**: pass it,
  store it, iterate it. No allocation; `for x in range { }` consumes it.

### Container literals — **Decided**
- **Array literal:** `[1, 2, 3]` — **map literal:** `[key: value, ...]`
  (a top-level `:` on the first element selects map mode; mixing shapes is a
  compile error).
- Elements are **arbitrary expressions** — calls, nested containers,
  `if`/`switch` expressions — evaluated left to right, exactly once.
- **Spread:** `[a, ...b]` / `[k: v, ...m]` flattens an array/slice (map) into
  the literal.
- **Contextual typing** from the target:
  - target `T[N]` — exactly `N` elements required (compile error otherwise);
  - target `Array<T>` — growable result;
  - no annotation — infers `Array<T>` (escape analysis may keep it on the
    stack if it never escapes/resizes);
  - empty `[]` requires a target type.
- Usable in **any position, expression or statement**; a bare literal as a
  standalone statement is legal but warns (unused value).
- The same bracket syntax is a **slice pattern** in `switch` prongs —
  patterns are more than expressions: `[first, ...rest] => ...`.

### Sum types
- **`variant`** — Rust-style enum with payloads:
  `variant Shape { Circle(f64), Rect(f64, f64) }`
- **Construction:** qualified `Shape.Circle(1.0)`; contextual `.Circle(1.0)`
  wherever the target type is known (assignment, typed argument, return).
- Built-in `Option<T>` / `Result<T, E>` are written with `variant` —
  **there is no `null`**.

### Type aliases
- `type Meters = f32;` — **transparent** aliases (zero cost, no new nominal
  type; interchangeable with the right-hand side).

### No user-defined generics — **Decided (removed)**
Mu has **no user-defined generics**: no type parameters on `fn`, `struct`, or
`variant`, no trait bounds, no turbofish, no monomorphization. Mu is a game
scripting language; generics were cut to keep the language learnable and the
compiler small. Writing `fn max<T>(a: T, b: T) T` is a parse error (**E0226**)
pointing at the `<` — write one function per type instead.
Built-in containers keep their type arguments (`Array<f32>`,
`Map<string, i32>`, `Option<Entity>`), because they are parameterized built-in
types, not user generics.

### Inference
- **Locals only**: `var x = 5;` infers; parameters and return types are
  always annotated. `let` (local constant) requires an initializer.
- **Definite assignment**: `var x: i32[3];` is uninitialized; reading before
  assignment is a compile error (no runtime cost, no zero-fill).

## 4. Value semantics — **Decided**

- **Mutable by default**: `var` is the ordinary binding and needs no marker
  to mutate; `let`/`const` opt into immutability.
- **Copy by default** for assignment/arguments; the compiler elides copies
  (move) when the source is not used again.
- **Single ownership.** Two owners of one heap value ⇒ move or copy; the
  compiler rejects double-free shapes. No shared ownership in v1.

## 5. Slice borrow rule — **Decided**

Because slices are views over memory owned by someone else:

1. A slice **must not outlive** its backing storage (compile error).
2. While a slice is live, the **owner is frozen** — it cannot be read, written,
   or moved until the slice's last use (non-lexical lifetimes).
3. While frozen, other slices of the same buffer remain usable.

Zero-cost views: no runtime checks, no dangling pointers, reusing the
lifetime analysis the compiler already performs for scope-based frees.
**No explicit borrow syntax exists or is planned** — the rule is entirely
compiler-internal; there is nothing for the programmer to write.

## 6. Memory model — **Decided**

- **No GC, no reference counting.**
- Compile-time stack/heap differentiation; the compiler **auto-injects
  alloc/free** calls.
- **Scope-based free + escape analysis**: values die at end of owning scope;
  escaping values are heap-allocated with ownership transferred.
- Backed by a **mimalloc-based allocator**.
- **Drop trait, not manual cleanup:** types that own resources implement
  the inbuilt **`@drop`** (a `drop()` method — the same trait Coral's std
  uses); the injected scope-free runs it automatically — **the leak
  guard**: owned resources are released at scope end even on early
  returns, so nothing waits for a hand-written free that might be
  forgotten. Scripts never write alloc/free/drop calls by hand.

### Closure capture (consequence of §4/§6)
- The compiler decides per captured variable:
  - closure provably **does not outlive** the variable's scope → capture by
    reference (zero copy, mutations visible);
  - closure **escapes** → capture by value (copy, or move when the owner dies).
- No dangling captures; no capture-syntax burden on the programmer.

## 7. Control flow — **Decided**

- `if` / `else`, `while`, `for ... in ...`, `break` / `continue` with
  `@`-marked labels (§1).
- **Expressions *and* statements:** `if`/`else`, `while`, and `switch` work
  in **either** position — never forced to be expression-only.
- **`if`/`else` branches are always braced blocks**, in either position:
  `var label = if score >= 90 { "A" } else if score >= 80 { "B" } else { "C" };`
  As an expression the block's **tail expression** is its value (all branches
  must produce one common type); as a statement the value is discarded.
  (`for`, `var`, `let`, `break`, `continue` are statements.)
- **`switch` — Zig-style:**
  - an **expression or statement**
    (`let y = switch (x) { ... };` or bare, with block prongs),
  - matches on **more than integers** — general value comparison plus
    patterns that destructure `variant` payloads, tuples, and slices,
  - **exhaustive** — all cases required unless `else` is present,
  - prongs are `pattern => expr,` separated by commas — **no `case`, no
    `break`**,
  - **`else` is the fallback prong** (required when cases cannot cover all
    values: integers, strings, floats),
  - **guards** attach conditions: `Pattern if cond => expr,`.
- **Patterns:** literals, ranges (`0..=9 =>`), variant payloads
  (`Rect(w, h)`), tuples (`(a, b)`), slices (`[first, ...rest]`), bindings
  (`x` matches and binds), `_` (ignore), or-patterns (`A | B =>`), plus
  `else`.
- **Float scrutinees must include `else`** — float equality is not a
  partition (NaN); exhaustiveness is not provable.
- Examples:
  ```
  var name = switch (x) {
      0                    => "zero",
      1, 2                 => "small",
      3..=9 if x % 2 == 0  => "even under ten",
      else                 => "big",
  };

  var area = switch (shape) {
      Circle(r)  => 3.14159 * r * r,
      Rect(w, h) => w * h,
  };
  ```

### `for` loops and iterators — **Decided**
- `for x in a..b { }` — ranges (exclusive `..`, inclusive `..=`).
- `for x in collection { }` — arrays, slices, `Array<T>`, `Map<K,V>`,
  `string`/`str`, `Range<T>`.
- Bindings may destructure: `for (k, v) in map { }`.
- **Iterator protocol is structural:** a type works in `for` if it has
  `fn next(self) Option<T>` (containers provide `iter()`).
- `for` desugars to one local iterator + `while` — monomorphized, inlinable,
  **zero heap allocation** for built-in containers.
- Iterator invalidation: mutating an `Array<T>` while a live iterator holds it
  is a compile error (the iterator holds a §5 view of the buffer).

## 8. Errors and panics — **Decided**

- Result-style explicit only. No exceptions, no unwinding.
- **`?` operator**: `foo()?` early-returns the `Err` variant; allowed only in
  functions returning `Result<...>`.
- **Panics** are unrecoverable per script context. They fire on:
  - index out of bounds (when not provably in range),
  - division/modulo by zero,
  - overflow when the overflow-trap build flag is on (§14),
  - failed `unwrap`-style access, invalid `u32 as char`,
  - script stack overflow (host-configurable limit; never UB),
  - explicit `panic("msg")`.
- The host installs a **panic handler** through the binding API: it receives
  the message, file/line, and script stack trace. Default: print and stop the
  context. No in-language `catch` in v1.

## 9. Traits and dispatch — **Decided**

- **Structural conformance:** if a type is put in a place where a trait is
  expected (parameter, field, generic bound), the compiler checks it has the
  required methods — **no `implements` declaration needed**.
  (`extend T: trait X { ... }` adds missing methods or names a conformance.)
- **Trait-object annotation is `dyn Trait`**:
  `fn draw(shape: dyn Renderable)` — explicit so it never collides with type
  names. Passing a conforming concrete type where `dyn Trait` is expected
  **coerces silently** (it "just fits"); the value moves to the heap and the
  fat pointer `{ data, vtable }` is formed (escape analysis may inline the
  store). Generic bounds never box — they are compile-time checks.
- Trait calls go through the **vtable**; the compiler **devirtualizes**
  (direct call/inline) whenever the concrete type is known.
- Traits may contain method signatures and **default method bodies** (inlined
  at instantiation). No supertraits in v1.

## 10. Operators and indexing — **Decided**

### Precedence, high → low (C-like)
```
postfix:   f()  a[i]  .  ?  ::<>
unary:     -x  !x  as
* / %
+ -
<< >>
&
^ |
== != < > <= >=     (non-associative: a < b < c is a type error)
&&
||
.. ..=
=  +=  -=  *=  /=  %=  &=  |=  ^=  <<=  >>=   (right-assoc)
```
- Comparisons yield `bool`; they cannot chain (bool < T fails to typecheck).
- `x += y` desugars to a single-evaluation assignment.

### Operator overload resolution (structural, in order)
1. User method, if the type defines it:
   `add sub mul div rem bitand bitor bitxor shl shr not neg eq ne lt gt le ge`
   (e.g. `fn add(self, other: Vec2) Vec2`).
2. Builtin: primitives, `char`, `bool`, `string` (`+` concatenates,
   comparisons are bytewise lexicographic).
3. Aggregates (structs, tuples, `variant`s): comparisons and `hash` resolve
   **field-wise** when every field resolves — compiled to a straight-line,
   fully inlinable comparison.
4. Otherwise: compile error. Arithmetic has no field-wise fallback.

### Indexing
- `[]` overloading is **builtin-only in v1** (arrays, slices, `Array<T>`,
  `Map<K,V>`); user types expose `.get()`/`.set()` methods.
  (Overloadable `[]`/`[]=` deferred.)
- Reads: `a[i]`, `m[k]` **panic** when out of bounds / key missing.
  Checked accessors exist: `a.get(i) -> Option<T>`, `m.get(k) -> Option<V>`.
- Writes: `a[i] = v`, `m[k] = v` — map write **inserts or assigns**.
- **Bounds checks are always emitted unless the compiler proves the index
  in range** (loop induction, fixed-array literals, known slice ranges).

## 11. Functions and closures — **Decided**

- Functions are first-class values; closures capture per §6.
- Overloading: **none within a module/scope** (§2). **Different modules may
  define the same function names** — call them through their module qualifier
  or alias (`render::draw(x)`, `audio::draw(y)`). When several `use`d modules
  bind the same bare name, a bare call resolves by signature (arity + types,
  all compile-time); ambiguity is a compile error.
- No default/named parameters in v1 (pass a struct if the list grows).
- Recursion allowed; tail calls are not guaranteed.

## 12. Async — **Decided**

- **One mechanism:** `async fn` — the coroutine *is* an async function; there
  is no separate coroutine keyword or generator type.
- Compiles to a **stackless state machine**; the frame is heap-allocated
  (mimalloc).
- Host **polls futures**; the engine drives polling from the game tick.
- `await` on engine-provided signals: timers, events, frame boundaries.
- `async` applies to `fn` (including methods); gameplay flows (cutscenes,
  input waits) are written as `async fn` sequences.
- **Cancellation:** futures are not individually cancellable in v1 — the
  host discards a context's pending async when it tears the context down
  (cutscene skipped, owner despawned).

## 13. Modules and visibility — **Decided**

- One module per file; module path derives from file path.
- **`pub`** marks exported symbols; everything else is file-private.
- **`use` bindings (Rust-style) — module paths use `::`:**
  ```
  use Mu::render;                 // binds `render`  -> render::draw(...)
  use Mu::render::draw as d;      // alias           -> d(...)
  use Mu::{render, physics};      // group           -> two bindings
  use Mu::render::{draw, Sprite}; // nested group
  ```
  The engine's own modules live under the root module **`Mu`** (its one
  capitalization exception).
  **`::` is path resolution; `.` is exclusively member/method access** —
  `render::draw()` (module path) and `value.draw()` (method call) can never
  be confused.
  Full paths resolve too (`Mu::render::draw(...)`) as long as each module
  along the path is `pub`. Two `use` statements binding the same name are a
  compile error — alias to disambiguate (or rely on signature resolution,
  §11). Same-named functions may live in any number of modules.
- **Extension activation:** `extend` blocks from a `use`d module are active
  in the using file; non-`pub` extensions apply only within their own
  file.
- **`const` items** (incl. `pub const`) are compile-time constants, inlined.
- **No mutable globals in v1** — mutable state lives in engine-owned or
  script-created structs the host holds.

### Symbol resolution and forward references — **Decided**

No forward declarations anywhere. Module imports may be **cyclic** — a file
may `use` a file that (transitively) `use`s it back. References resolve
**on demand** through a dependency graph:

- referenced symbol already evaluated → done;
- not yet evaluated → record *referrer depends on ref* and evaluate the ref
  (the referrer resumes when it finishes);
- file not loaded yet → record the pending reference; loading that file
  resumes everything that waited on its symbols.

**Declarations are known before evaluation:** every module's items (`fn`,
`struct`, `variant`, `const`, `type`, `extend`) register their signatures
when the file is parsed — only bodies and values are evaluated on demand.
Mutual recursion therefore needs no forward declaration: `fn a() { b(); }`
beside `fn b() { a(); }` compiles. Within a function body, locals stay
strictly sequential — a `var`/`let` exists only after its declaration.

Whatever is still waiting when evaluation ends is a cycle, reported with
the chain spelled out:
```
const A: i32 = B;  // error: cyclic dependency: A -> B -> A
const B: i32 = A;
```

## 14. Implementation — **Decided**

- Compiler written in **Coral (`.crl`)**, compiled with coralc — no
  templates, no classes, no C++ anywhere: modules, structs + in-body
  methods, `extend` blocks, and built-in generics only. It uses Coral's own
  standard library (`Vec<T>`, `Arr<T,N>`, `alloc<T>`/`dealloc<T>`, `str`) —
  no external dependencies. **Allocation is an engine-side concern:** the
  embedding engine supplies the mimalloc-based allocator at runtime.
- **Single-pass front end:** the *grammar* is unambiguous by construction —
  one-pass recursive descent, no backtracking. Where two readings could
  collide, the language pins one down (e.g. the control-flow-head rule, §2;
  `@`-marked labels and `@extern`, §1). Symbol resolution is a separate,
  demand-driven step (§13).
- Pipeline: source → **custom SSA IR (spec comes later)** → AOT/JIT to native.
  - **Dev:** compile at load, hot reload supported, result cached.
  - **Ship:** AOT to native.
- **Hot reload policy:** pure-script heap state does **not** survive a reload.
  Persistent state lives in engine-side objects held by the host; after a
  reload the host re-attaches them through the binding table.
- **Save/load:** saves are host-side; pure-script heap state is not a save
  mechanism. The host serializes engine-owned objects plus a script data
  blob through the binding table.
- Engine ↔ script interop: **manual binding API.** All engine built-ins
  (`print`, math, …) are always in scope — scripts never declare them.
  This is **prelude-style** (like Rust's prelude): an explicit
  `use Mu::<module>::<fn>;` (§2) of a built-in is legal and idiomatic —
  it changes nothing semantically; it records which library symbols the
  script depends on and enables `{...}` grouping and `as` aliases.
  Built-in *calls* stay bare identifiers — `@` never starts an expression
  (§1), so there is no `@call` form; `@` remains declaration-space
  (attributes, loop labels, the `@extern` modifier).
  Foreign functions the engine does not provide (C/C++ etc.) are declared
  **`@extern fn ...;`** (no body) and registered by the host with a matching
  name, module path, and signature; the loader verifies the match at load
  and fails loudly on mismatch.
- **Engine objects are opaque handles with generation checks** — using a
  despawned/freed object panics (same class as bounds checks, §8), never
  silent corruption.
- **Script manifest:** directives on lines starting `#!` before any code:
  ```
  #! string.casing, string.format
  ```
  Controls which built-in string method groups are compiled in for this
  script (§3). Unknown groups are a compile error. The directive form is the
  home for future per-script flags.
- **Numeric overflow wraps** by default (one instruction, defined behavior
  across builds); a debug build flag enables overflow trapping. Checked
  variants (`checked_add` etc., returning `Option<T>`) are built in.
- **Panic handler registration** (§8) is part of the binding API.

## 15. Engine runtime rules — **Decided**

- **Threading:** a script context runs only on the thread that polls it;
  every engine callback for that context arrives on that same thread. No
  script-visible threads or locking in v1. The host may poll different
  contexts on different threads — each context is single-threaded.
- **Determinism (replays / lockstep):**
  - no per-run hash randomization: `Map` uses fixed-seed hashing — the same
    insertion sequence yields the same iteration order on every run and
    platform;
  - floating point is strict IEEE-754 — no fast-math, no FMA
    reassociation;
  - the engine RNG is a seedable built-in.
  Same seed + same inputs ⇒ same simulation everywhere.

---

## 16. Attributes — **Decided**

Attributes are `@name` markers written immediately before a declaration:

```
@component struct Health { hp: f32; max: f32; }
@system(update) fn regen(g: &World) { ... }
@test fn assert_hp() { ... }
@extern fn platform_log(s: &str);
```

- **Placement:** attributes follow any `pub`/`async` modifiers and apply to
  the single declaration that follows; a member may carry its own attributes
  (`@system` on a struct method, `@doc` on a field group). `@extern` is a
  **modifier**, not an attribute — it is valid only directly before `fn`.
- **The v1 set:**
  - `@component`, `@event`, `@save` — `struct` or `variant`, no arguments.
  - `@resource` — `struct`, no arguments.
  - `@system` — `fn`; 0 or 1 argument: a phase path from `startup`,
    `pre_update`, `update`, `fixed_update`, `post_update`, `render`,
    `shutdown` (no argument means `update`).
  - `@inline`, `@noinline`, `@must_use`, `@test` — `fn`, no arguments.
  - `@repr` — `struct`; exactly one argument: `c` or `packed`.
  - `@deprecated` — named declaration; optional string argument.
  - `@doc` — named declaration; exactly one string argument.
- **Arguments** are literals or bare paths only — never expressions.
- **Diagnostics:** unknown attribute E0219, duplicate attribute E0220, wrong
  arity or argument kind E0221, wrong placement E0222, unknown phase E0223,
  duplicate/`@extern`-placement modifiers E0215.
- Attributes ride along on the AST (`Decl.attrs`); the parser validates name,
  count, placement, and argument shape — interpretation (ECS registration,
  `@test` harness, `@repr` layout, `@must_use` flow) belongs to later phases.

---

## Open items

- [ ] **SSA IR spec** — pending from the author. (Draft for review: `IR.md`.)

## Deferred to later versions (consciously postponed, not open)

- Shared ownership (`shared`/`weak` handles) and cycle concerns.
- Supertraits.
- Overloading within a module/scope (cross-module same names already work,
  §11).
- Overloadable indexing (`[]` / `[]=` on user types).
- Mutable globals.
- In-language recoverable panics / `catch`.
- Separate coroutine/generator type distinct from `async fn`.
