# Mu tooling protocols — compiler ↔ IDE

The Mu compiler (`muc`, in `Mu_coral/`) has three output modes besides the
human caret report. They exist so editors can consume diagnostics without
parsing text, and so the engine's in-game test editor can read them with a
tiny parser.

| mode | flag | shape | who reads it |
|---|---|---|---|
| JSON document | `muc --json file.mu` | one JSON object, LSP-compatible diagnostics | scripts, CI, `jq`, editors that run the compiler directly |
| Compact buffer | `muc --buf file.mu` | tab-separated records, one per line | the engine's in-game test editor |
| Language server | `muc --lsp` | JSON-RPC 2.0 over stdio | Zed, VSCode, any LSP client |

All three come from the same source: `src/mu-json/`.

- `json.crl` — turns `Diags` records into JSON using the standard library's
  JSON writer (`std::parser::json`), so the compiler never hand-rolls
  escaping.
- `buf.crl` — the compact format, hand-written with no intermediate tree.
- `lsp.crl` — the stdio loop: framing, request parsing (again via
  `std::parser::json`), re-running the front end on the in-memory document,
  and `publishDiagnostics`.
- `src/mu-frontend/analyze.crl` — the shared "lex → parse → check" entry point
  so the one-shot modes and the server cannot drift apart.

Diagnostics are recorded, not printed: `Diags` keeps a `Vec<DiagRec>` with
severity, code, position, length, title and notes, and any human output is
suppressed (`diags.quiet`) while a machine mode is active so stdout carries
only the payload.

## 1. `--json`

```json
{
  "file": "scene.mu",
  "errors": 1,
  "warnings": 0,
  "diagnostics": [
    {
      "range": { "start": { "line": 6, "character": 0 },
                 "end":   { "line": 6, "character": 5 } },
      "severity": 1,
      "source": "muc",
      "code": "E0350",
      "message": "unknown name\nwhy: nothing named `power` here"
    }
  ]
}
```

Positions are 0-based (LSP convention) — the compiler's 1-based line/column is
shifted on the way out. `severity` is `1` for errors and `2` for warnings.
`code` is the compiler's diagnostic code (`E0350`, `E0400`, …), which is also
the key into `errors.md`.

Exit status is still non-zero when the program has errors, so scripts can use
either the JSON or the status.

## 2. `--buf` — the engine test editor stream

One record per line, tab-separated, no quoting and no escaping at all:

```
H  scene.mu 1 0 1
E  7 5 3 E0350 unknown name
N  why: nothing named `power` here
```

```
H <file> <errors> <warnings> <record-count>
E <line> <col> <len> <code> <title>
W <line> <col> <len> <code> <title>
N <note>                       (follows its record)
```

It is deliberately smaller and faster than JSON for that use: the editor's
status line parses it with a scan for tabs, there is no allocation per
diagnostic, and the whole report is written with one `write(2)`. Messages and
notes keep their text verbatim — a note containing a tab or newline would lose
that byte, which no compiler diagnostic does today.

## 3. `--lsp` — the language server

stdio JSON-RPC 2.0 with the usual LSP framing:

```
Content-Length: <byte count>\r\n\r\n<json body>
```

Supported methods:

| method | direction | behavior |
|---|---|---|
| `initialize` | client → server | replies with `{ "textDocumentSync": 1 }` (full-text sync) |
| `initialized` | client → server | ignored |
| `textDocument/didOpen` | client → server | stores the buffer, re-analyzes, publishes diagnostics |
| `textDocument/didChange` | client → server | full-text replace, re-analyzes, publishes |
| `textDocument/didClose` | client → server | publishes an empty diagnostic list |
| `shutdown` | client → server | replies `null` |
| `exit` | client → server | ends the loop |

Anything else with an `id` gets a `-32601` method-not-implemented reply, so a
client probing `textDocument/hover` or `completion` fails fast instead of
hanging until it times out. Notifications without an `id` that are not handled
are simply ignored, per the spec.

Sync is full-text (`DocumentSyncKind.Full`), so `didChange` uses the last
`contentChanges` entry. The document lives in memory only; nothing is written
to disk.

### Using it with an editor

Zed (`settings.json`):

```json
{
  "lsp": {
    "mu": {
      "command": "muc",
      "args": ["--lsp"]
    }
  }
}
```

VSCode — any extension that lets you configure a command-based language
server (the command above is the whole configuration) or a thin launcher that
spawns `muc --lsp`.

The bundled Zed extension at `Mu/editors/zed/` is **syntax highlighting only**
and does not register this server; add the snippet above if you want
diagnostics.

## Deliberate limits

- One document at a time; no workspace, no cross-file symbol index.
- No hover, completion, go-to-definition, or semantic tokens yet — the
  compiler does not have an IR module to answer them from. Diagnostics only.
- `initialize` advertises no capabilities beyond full-text sync, so well-behaved
  clients will not ask for the rest.
