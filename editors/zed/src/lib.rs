// Mu extension: syntax highlighting only, via the tree-sitter grammar in
// grammars/mu. The engine's in-game editor and third-party IDEs get their
// tooling from the compiler itself (`muc --json` / `--buf` / `--lsp`), which
// is deliberately not wired here — see editors/zed/README.md for the manual
// settings snippet if you want diagnostics too.
use zed_extension_api as zed;

struct MuExtension {}

impl zed::Extension for MuExtension {
    fn new() -> Self
    where
        Self: Sized,
    {
        MuExtension {}
    }
}

zed::register_extension!(MuExtension);
