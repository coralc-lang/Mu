# Mu <-> Coral bindings

**Fast path:** compiler-emitted `u64` symbol hash -> `Binding`. No runtime
source parsing, name comparison, demangling, or type-source scanner.

## Contract

- **Symbol identity is compiler-owned.** The Mu compiler emits the canonical
  symbol for each function/type (it owns mangling, modules, generics, and
  signatures). The engine treats the exact emitted bytes as opaque.
- **Hash:** FNV-1a 64 over those exact bytes.
  - offset basis `0xcbf29ce484222325`
  - prime `0x100000001b3`
  - fixed seed, no per-run randomization
  - implementation: `src/core/hash.crl` (`hashBytes`, `hashStr`, `hashCstr`)
- **No hand-written `Mu::...` strings at the ABI boundary.** Those are source
  paths, not canonical symbols. Registration uses compiler-generated hash
  constants.
- A function hash covers the full signature. Calling with the wrong signature
  yields a different hash, so lookup is simply `NotFound`.

## Coral side

`src/script/abi.crl` owns the table:

```coral
pub variant Binding
{
    Function { Native native },   // one arm per boundary signature
    Type { u32 size; u32 align; }
}
```

- `BindingTable` is open-addressed and uses caller-provided static storage
  (like `Scene` and `ScriptBindings`).
- `addFunction(hash, native)` / `addType(hash, size, align)` register once.
- `lookup(hash)` probes by hash only and returns `Binding*` or `null`.
- Duplicate hash or full table returns `Result<_, str>` at registration.

The table is optional safety/diagnostics. Directly linked symbols still work
without it. It is useful for collision detection, hot reload, tools, and type
layout metadata.

## Lookup

```coral
Binding* item = bindings.lookup(symbolHash);
if (item == null) return NotFound;
switch (item[0])
{
    Function { native } => dispatch(native),
    Type { size, align } => useLayout(size, align)
}
```

The compiler emits `symbolHash` as a literal in the generated binding/IR, so
the runtime path is one hash probe plus the typed switch.

## What is deliberately absent

- No `.mu` source reader in the host.
- No `use` parser or alias expansion at runtime.
- No runtime Mu type-source scanner or declaration table.
- No string lookup on the hot path.

Those are compiler responsibilities. `ScriptHost.load`/`update`/`free` remain
the IR/lifecycle seam for `muc`.
