// Validate the generated TextMate grammar with VS Code's own textmate engine.
// Uses the copies already installed under /usr/lib/code/node_modules, so this
// needs no network access.
//
//   node tools/check-tmgrammar.js [file.crl ...]

const fs = require('fs');
const path = require('path');

const TM = '/usr/lib/code/node_modules/vscode-textmate';
const ONI = '/usr/lib/code/node_modules/vscode-oniguruma';

const oniguruma = require(path.join(ONI, 'release/main.js'));
const vsctm = require(path.join(TM, 'release/main.js'));

const GRAMMAR = path.resolve(__dirname, '../syntaxes/mu.tmLanguage.json');

// vscode-oniguruma is a WASM module and must be initialised before use.
const onigWasm = fs.readFileSync(path.join(ONI, 'release/onig.wasm'));

const registry = new vsctm.Registry({
  onigLib: oniguruma.loadWASM(onigWasm.buffer).then(() => ({
    createOnigScanner: (s) => new oniguruma.OnigScanner(s),
    createOnigString: (s) => new oniguruma.OnigString(s),
  })),
  loadGrammar: async (scope) => {
    if (scope === 'source.mu') {
      const g = fs.readFileSync(GRAMMAR, 'utf8');
      return vsctm.parseRawGrammar(g, GRAMMAR);
    }
    return null;
  },
});

async function tokenize(file) {
  const grammar = await registry.loadGrammar('source.mu');
  if (!grammar) throw new Error('grammar failed to load');

  const text = fs.readFileSync(file, 'utf8');
  let ruleStack = vsctm.INITIAL;
  const lines = text.split(/\r?\n/);
  const out = [];

  for (let i = 0; i < lines.length; i++) {
    const r = grammar.tokenizeLine(lines[i], ruleStack);
    for (const t of r.tokens) out.push({ line: i + 1, ...t });
    ruleStack = r.ruleStack;
  }
  return out;
}

function main() {
  const files = process.argv.slice(2);
  if (!files.length) {
    console.error('usage: node tools/check-tmgrammar.js <file.crl ...>');
    process.exit(2);
  }
  (async () => {
    let total = 0;
    for (const f of files) {
      let toks;
      try {
        toks = await tokenize(f);
      } catch (e) {
        console.error(`FAIL ${path.basename(f)}: ${e.message}`);
        process.exitCode = 1;
        continue;
      }
      const scopes = new Set(toks.map((t) => t.scopes.filter((s) => s !== 'source.mu')[0]).filter(Boolean));
      total += toks.length;
      console.log(`ok   ${path.basename(f)}  ${toks.length} tokens, ${scopes.size} distinct scopes`);
      if (process.env.VERBOSE) {
        for (const t of toks) {
          const line = fs.readFileSync(f, 'utf8').split(/\r?\n/)[t.line - 1];
          console.log(`   ${t.line}:${t.startIndex}-${t.endIndex} ` +
            `${JSON.stringify(line.slice(t.startIndex, t.endIndex))} ` +
            t.scopes.join(' '));
        }
      }
    }
    console.log(`\ntotal ${total} tokens`);
  })();
}

main();
