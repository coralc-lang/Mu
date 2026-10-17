#!/usr/bin/env node
// Check every node type and field name used in a .scm file against the
// grammar's generated node-types.json.
//
// Tree-sitter reports these one at a time, which is slow when a query drifted
// from the grammar. This reports all of them, and is what the install step
// should gate on: one bad reference makes Zed refuse the whole language, so
// highlighting silently disappears.
//
//   node tools/check-queries.js languages/mu/*.scm

const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const TYPES = path.join(root, 'grammars/mu/src/node-types.json');

function loadTypes() {
  const nodes = JSON.parse(fs.readFileSync(TYPES, 'utf8'));
  const types = new Set();
  const fields = new Map();          // node -> Set(field)
  const supertypes = new Set();
  for (const n of nodes) {
    if (n.named) types.add(n.type);
    if (n.subtypes) supertypes.add(n.type);
    if (n.named && n.fields) fields.set(n.type, new Set(Object.keys(n.fields)));
  }
  // anonymous tokens are legal in queries too
  for (const n of nodes) if (!n.named) types.add(n.type);
  return { types, fields, supertypes };
}

function check(file, { types, fields }) {
  const text = fs.readFileSync(file, 'utf8');
  const problems = [];

  // strip comments and strings so their contents are not treated as node names
  const body = text
    .replace(/;[^\n]*/g, '')
    .replace(/"(?:[^"\\]|\\.)*"/g, '""');

  // (node_type  ...   and   field_name:
  const nodeRe = /\(\s*([A-Za-z_][A-Za-z0-9_]*)/g;
  let m;
  while ((m = nodeRe.exec(body))) {
    if (!types.has(m[1])) {
      problems.push(`unknown node type "${m[1]}"`);
    }
  }
  const fieldRe = /\b([A-Za-z_][A-Za-z0-9_]*)\s*:/g;
  const seenFields = new Set();
  while ((m = fieldRe.exec(body))) {
    const f = m[1];
    if (!seenFields.has(f)) {
      seenFields.add(f);
      if (![...fields.values()].some((s) => s.has(f))) {
        problems.push(`unknown field name "${f}:"`);
      }
    }
  }
  return problems;
}

function main() {
  const { types, fields } = loadTypes();
  let bad = 0;
  for (const file of process.argv.slice(2)) {
    const problems = check(file, { types, fields });
    const name = path.basename(file);
    if (!problems.length) {
      console.log(`ok   ${name}`);
    } else {
      bad++;
      console.log(`FAIL ${name}`);
      for (const p of problems) console.log(`       ${p}`);
    }
  }
  process.exit(bad ? 1 : 0);
}

main();
