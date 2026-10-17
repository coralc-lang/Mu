# Mu for Zed

Syntax highlighting for [Mu](https://github.com/coralc-lang/Mu) — a
tree-sitter grammar plus the bracket/indent/outline queries that come with it.
Nothing else is bundled: no language server, no formatter.

## Install (dev)

```sh
cd editors/zed/grammars/mu
npm install
npx tree-sitter generate
npx tree-sitter build
npx tree-sitter test          # runs grammars/mu/test/corpus/*.txt
```

Then in Zed: `Cmd+Shift+P` → `zed: install dev extension` → pick
`editors/zed`.

To package it for sharing:

```sh
cd editors/zed
npm i -g zed-extension-cli     # or: cargo install zed-extension
zed-extension build
```

## What the grammar covers

- declarations: `fn`, `struct`, `variant`, `trait`, `extend`, `type` aliases,
  `const`, `use` (with `{ a, b }` groups and `as`), attributes `#[[...]]`
- statements: `var`/`let`, `if`/`else`, `while`, `for x in y`, `switch` with
  `pattern => expr` prongs and guards (`value if cond => ...`), `return`,
  `break`/`continue` with `@label`s
- expressions: calls (`f(x)`, `x.m(y)`, `T::new(x)`), field/index access,
  `a as T`, `await`, `?`, unary/binary/assignment operators, ranges (`..`,
  `..=`), spreads (`[...b]`), array/map/tuple/struct literals
- literals: ints with `_` separators and hex/bin, `42i8`-style suffixes,
  floats, `'μ'` and `'\u{1F600}'` chars, strings, `true`/`false`
- types: `T[N]`, `T[]`, `(A, B)`, `Array<T>`, `Map<K, V>`, `Set<T>`,
  `Queue<T>`, `Pool<T>`, `dyn Trait`
- comments: `//` and `/* */`

## Diagnostics (optional, not wired here)

The extension deliberately ships no language server. If you want errors in the
editor, point Zed at the Mu compiler, which speaks LSP over stdio:

```json
{
  "lsp": {
    "mu": {
      "command": "muc",
      "args": ["--lsp"]
    }
  },
  "languages": {
    "Mu": { "formatter": null }
  }
}
```

`muc --json file.mu` prints the same diagnostics as one JSON document, and
`muc --buf file.mu` prints a compact tab-separated stream used by the engine's
in-game test editor. See `Mu_docs/LSP.md`.

## Layout

```
editors/zed/
  extension.toml           zed manifest (grammar only)
  Cargo.toml               wasm crate — empty, no language server
  src/lib.rs
  grammars/mu/
    grammar.js             the Mu grammar
    tree-sitter.json
    package.json
    test/corpus/           highlight/parse corpus
  languages/mu/
    config.toml            suffixes, comment syntax, brackets, tab size
    highlights.scm         syntax highlighting
    indents.scm           indentation
    outline.scm           outline view
    textobjects.scm       selection by scope
```
`languages/mu/config.toml` declares the bracket pairs and comment syntax.
