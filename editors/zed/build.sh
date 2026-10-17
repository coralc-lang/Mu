#!/usr/bin/env bash
# Build the Mu tree-sitter grammar to wasm and install the Zed extension.
#
# Fully offline: the generated parser makes no libc calls, so stub headers
# stand in for stdlib.h / string.h / assert.h / stdio.h, and wasm-ld emits the
# `dylink` section Zed's wasm loader requires.
#
# Every query is compiled against the real grammar before installing. A single
# bad node type, field name, or impossible pattern makes Zed refuse the whole
# language, so highlighting silently disappears — this is the gate that stops
# that shipping.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
G="$ROOT/grammars/mu"
DST="${ZED_EXT_DIR:-$HOME/.local/share/zed/extensions/installed/mu}"
INC="$(mktemp -d)"
PROBE="$(mktemp -t mu-probe.XXXXXX.crl)"
trap 'rm -rf "$INC" "$PROBE"' EXIT

printf '#include <stddef.h>\n'                            > "$INC/stdlib.h"
printf '#include <stddef.h>\n'                            > "$INC/string.h"
printf '#define assert(e) ((void)0)\n'                     > "$INC/assert.h"
printf '#include <stddef.h>\ntypedef struct _FILE FILE;\n' > "$INC/stdio.h"

echo '== clang -> wasm object'
clang --target=wasm32-unknown-unknown -fPIC -O2 -fno-builtin \
      -I"$INC" -I"$G/src" -c "$G/src/parser.c" -o "$ROOT/mu-wasm.o" || exit 1

echo '== wasm-ld -> mu.wasm'
wasm-ld -shared "$ROOT/mu-wasm.o" -o "$ROOT/grammars/mu.wasm" \
        --export=tree_sitter_mu --strip-all 2>&1 | grep -v 'not yet stable' || true
rm -f "$ROOT/mu-wasm.o"
test -s "$ROOT/grammars/mu.wasm" || { echo 'wasm build failed'; exit 1; }

cat > "$PROBE" <<'EOF'
// probe exercising the query surface
use std::io;
type Pair = struct { first: i32, second: i32 }
trait Shape { fn area(self) -> f32; }
enum Color { Red, Green(u8), Blue }
extend Pair { fn sum(self) -> i32 { return self.first; } }
fn main() {
    let p = Pair { first: 1, second: 2 };
    let n = 0;
    if n > 1 { n = 2; } else { n = 3; }
    while n < 10 { n += 1; }
    for v in list { print(v); }
    switch n { 1 => { n = 0; }, _ => { n = 1; }, }
    let s = "hi"; let c = 'x'; let f = 1.5;
}
EOF

echo '== verify queries'
echo '   -- static check (node types / field names)'
node "$ROOT/tools/check-queries.js" "$ROOT"/languages/mu/*.scm | sed 's/^/   /' || exit 1

echo '   -- compile against the real grammar'
cd "$G" || exit 1
for q in brackets highlights indents outline textobjects; do
  printf '   %-16s' "$q.scm"
  if tree-sitter query "$ROOT/languages/mu/$q.scm" "$PROBE" >/dev/null 2>/tmp/.__mu_qerr; then
    echo OK
  else
    echo 'FAIL - refusing to install'
    head -5 /tmp/.__mu_qerr | sed 's/^/      /'
    rm -f /tmp/.__mu_qerr
    exit 1
  fi
done
rm -f /tmp/.__mu_qerr

echo '== install'
mkdir -p "$DST"
rsync -a --delete \
  --exclude '.git/' --exclude 'target/' --exclude 'test/' \
  --exclude 'build.sh' --exclude 'Cargo.toml' --exclude 'src/' \
  "$ROOT/" "$DST/"
ls -la "$DST/grammars/mu.wasm"
