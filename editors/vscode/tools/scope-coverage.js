// Report scope coverage for the generated TextMate grammar.
//
// A TextMate grammar cannot match a parse tree the way tree-sitter can, so
// this measures how much of a file actually receives a style, and which scopes
// carry the most tokens. Low `unscoped` and a long tail of scopes is the goal.
//
//   node tools/scope-coverage.js <file.crl ...>

const fs = require('fs');
const path = require('path');
const vsctm = require('/usr/lib/code/node_modules/vscode-textmate/release/main.js');
const oniguruma = require('/usr/lib/code/node_modules/vscode-oniguruma/release/main.js');

const GRAMMAR = path.resolve(__dirname, '../syntaxes/mu.tmLanguage.json');
const wasm = fs.readFileSync(
  '/usr/lib/code/node_modules/vscode-oniguruma/release/onig.wasm');

const registry = new vsctm.Registry({
  onigLib: oniguruma.loadWASM(wasm.buffer).then(() => ({
    createOnigScanner: (s) => new oniguruma.OnigScanner(s),
    createOnigString: (s) => new oniguruma.OnigString(s),
  })),
  loadGrammar: async (scope) =>
    scope === 'source.mu'
      ? vsctm.parseRawGrammar(fs.readFileSync(GRAMMAR, 'utf8'), GRAMMAR)
      : null,
});

const IGNORE = new Set(['source.mu']);

function deepest(scopes) {
  // innermost non-root scope that names a semantic class
  for (let i = scopes.length - 1; i >= 0; i--) {
    if (!IGNORE.has(scopes[i])) return scopes[i];
  }
  return null;
}

async function analyse(file) {
  const grammar = await registry.loadGrammar('source.mu');
  const text = fs.readFileSync(file, 'utf8');
  const lines = text.split(/\r?\n/);
  let stack = vsctm.INITIAL;
  const stats = { tokens: 0, unscoped: 0, byScope: new Map(), words: 0 };
  const unscopedSamples = [];

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const r = grammar.tokenizeLine(line, stack);
    stack = r.ruleStack;
    for (const t of r.tokens) {
      const body = line.slice(t.startIndex, t.endIndex);
      if (!body.trim()) continue;            // whitespace is not a signal
      stats.tokens++;
      const s = deepest(t.scopes);
      if (s) {
        stats.byScope.set(s, (stats.byScope.get(s) || 0) + 1);
        if (/[A-Za-z_]/.test(body)) stats.words++;
      } else {
        stats.unscoped++;
        if (unscopedSamples.length < 12) {
          unscopedSamples.push({ line: i + 1, text: body.trim() });
        }
      }
    }
  }
  return { stats, unscopedSamples };
}

async function main() {
  const files = process.argv.slice(2);
  if (!files.length) { console.error('usage: node tools/scope-coverage.js <file...>'); process.exit(2); }

  let T = 0, U = 0, W = 0;
  const merged = new Map();
  const samples = [];

  for (const f of files) {
    const { stats, unscopedSamples } = await analyse(f);
    T += stats.tokens; U += stats.unscoped; W += stats.words;
    for (const [k, v] of stats.byScope) merged.set(k, (merged.get(k) || 0) + v);
    for (const s of unscopedSamples) if (samples.length < 16) samples.push(s);
  }

  const styled = T - U;
  console.log(`files            ${files.length}`);
  console.log(`tokens           ${T}`);
  console.log(`scoped           ${styled} (${(100 * styled / T).toFixed(1)}%)`);
  console.log(`unscoped         ${U} (${(100 * U / T).toFixed(1)}%)`);
  console.log(`distinct scopes  ${merged.size}`);
  console.log('\nby scope:');
  for (const [k, v] of [...merged].sort((a, b) => b[1] - a[1])) {
    console.log(`  ${String(v).padStart(6)}  ${k}`);
  }
  console.log('\nunscoped samples:');
  for (const s of samples) console.log(`  ${s.line}: ${JSON.stringify(s.text)}`);
}

main();
