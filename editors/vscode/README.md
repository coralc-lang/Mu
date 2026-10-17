# Mu for VS Code

Syntax highlighting, bracket matching, and indentation for the
[Mu](https://github.com/coralc-lang/Mu) language. Mu files use the `.mu`
extension.

## Install

```sh
code --install-extension mu-0.1.0.vsix
```

## Layout

| Path | Purpose |
| --- | --- |
| `package.json` | Extension manifest. Registers the `mu` language and its TextMate grammar. |
| `language-configuration.json` | Comments, brackets, auto-closing pairs, indentation rules. |
| `syntaxes/mu.tmLanguage.json` | The TextMate grammar. **Generated — do not hand-edit.** |
| `gen-tmgrammar.py` | Generates the grammar from the tree-sitter grammar. |
| `tools/check-tmgrammar.js` | Tokenises real Mu files with VS Code's own textmate engine. |
| `tools/scope-coverage.js` | Reports how much of a file receives a style. |
| `tools/package-vsix.py` | Builds the `.vsix` with no npm downloads. |

## The grammar is generated

VS Code has no tree-sitter support, so highlighting needs a TextMate grammar.
Rather than maintaining a second copy of Mu's keywords by hand, this grammar is
generated from the tree-sitter grammar:

```
editors/zed/grammars/mu/grammar.js
        │  tree-sitter generate
        ▼
editors/zed/grammars/mu/src/grammar.json
        │  editors/vscode/gen-tmgrammar.py
        ▼
editors/vscode/syntaxes/mu.tmLanguage.json
```

Keyword and operator sets are lifted straight out of `grammar.json`, so the two
extensions cannot drift. Change a keyword and re-run:

```sh
python3 gen-tmgrammar.py
```

Two things a TextMate engine cannot infer from a parse tree, so the generator
emits them as patterns:

- **Pattern order.** TextMate takes the first match per span, so the identifier
  fallbacks (`@type` for `CapWords`, `@variable` for lowercase) are emitted
  **last**. With them first, every function, trait, struct, and field name
  collapses onto plain `@variable` and the file reads as one flat colour.
- **Position.** Line-anchored patterns catch declarations
  (`fn name(`, `struct Name`, `let x`, `name: T`), a `(` lookahead catches
  calls, and `.name` catches field and method access.

## Verifying

```sh
node tools/check-tmgrammar.js ../Mu_coral/mu-examples/traits.mu
VERBOSE=1 node tools/check-tmgrammar.js ../Mu_coral/mu-examples/traits.mu
node tools/scope-coverage.js ../Mu_coral/mu-examples/*.mu
```

`VERBOSE=1` prints every token with its scopes, which is how you confirm a
construct got the scope you intended. `scope-coverage` reports the fraction of
tokens that receive a style and how many distinct scopes are in play — a flat
one or two scopes is the symptom of the pattern-order bug above.

Both use the `vscode-textmate` and `vscode-oniguruma` copies already on the
system, so they need no network access.

## Current coverage

98% of tokens scoped, 36 distinct scopes, across the Mu example suite and the
engine's `main.mu`. Function names, method and field access, calls, parameters,
struct and trait names, `self`, literals, and operators are all distinguished.

## Notes

- The language server is deliberately not wired here. The compiler provides
  `muc --json` / `--buf` / `--lsp` for that; see `editors/zed/README.md`.
- No formatter or debugger.

## Licence

MIT
