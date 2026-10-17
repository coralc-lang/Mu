#!/usr/bin/env python3
"""Generate a VS Code TextMate grammar from the tree-sitter Mu grammar.

VS Code has no tree-sitter support, so highlighting there needs a TextMate
grammar. `tree-sitter` cannot emit one and the ecosystem hand-writes them,
which guarantees drift from grammar.js.

So this reads `src/grammar.json` - the artefact `tree-sitter generate`
produces - and lifts the keyword and operator sets straight out of it, then
appends the structural patterns a TextMate engine needs. Re-run after changing
grammar.js.

    python3 gen-tmgrammar.py

Ordering note: TextMate tries patterns in order and the FIRST match wins for a
given span, so the identifier fallbacks are emitted last. With them first,
every function, trait and struct name collapses onto plain @variable.
"""
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent
GRAMMAR = ROOT.parent / 'zed' / 'grammars' / 'mu' / 'src' / 'grammar.json'
OUT = ROOT / 'syntaxes' / 'mu.tmLanguage.json'

IDENT = r'[A-Za-z_][A-Za-z0-9_]*'


# --------------------------------------------------------------- extraction

def literals(rule):
    """Every STRING literal in a rule body."""
    found = set()

    def walk(node):
        if isinstance(node, dict):
            if node.get('type') == 'STRING':
                found.add(node['value'])
            for key, val in node.items():
                if key != 'type' or val != 'STRING':
                    walk(val)
        elif isinstance(node, list):
            for item in node:
                walk(item)

    walk(rule.get('members', rule))
    return found


class Extractor:
    def __init__(self, grammar):
        self.rules = grammar['rules']
        self.claimed = {}

    def take(self, role, names, drop=()):
        got = set()
        for name in names:
            body = self.rules.get(name)
            if not body:
                continue
            for tok in literals(body):
                if tok in drop or not tok.isidentifier() and tok.isalpha():
                    continue
                if self.claimed.setdefault(tok, role) == role:
                    got.add(tok)
        return got

    def keywords(self, rules, drop=()):
        """Keyword tokens, keeping the word-shaped ones only."""
        out = set()
        for name in rules:
            body = self.rules.get(name)
            if not body:
                continue
            for tok in literals(body):
                if tok in drop:
                    continue
                if not re.fullmatch(r'[a-z_][a-z0-9_]*', tok):
                    continue          # punctuation and operators handled apart
                out.add(tok)
        return out

    def operators(self, names):
        """Punctuation and operator tokens."""
        out = set()
        for name in names:
            body = self.rules.get(name)
            if not body:
                continue
            for tok in literals(body):
                if re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', tok):
                    continue
                if tok.startswith('#') or tok in ('(', ')', '[', ']', '{', '}'):
                    continue          # brackets are their own scope
                out.add(tok)
        return out


def alt(words):
    """Regex alternation, longest first so `<<=` beats `<<`."""
    ordered = sorted((w for w in words if w), key=len, reverse=True)
    return '|'.join(re.escape(w) for w in ordered)


def word(words):
    return r'\b(?:%s)\b' % alt(words)


# ------------------------------------------------------------------ emitting

def build(x):
    decl_kw = x.keywords((
        'function_definition', 'struct_definition', 'variant_definition',
        'trait_definition', 'extend_definition', 'type_definition',
        'const_definition', 'use_declaration', 'parameter',
    ), drop=('fn', 'self', 'true', 'false'))
    # `fn` opens a function; keep it with the other declaration keywords.
    decl_kw.add('fn')

    control_kw = x.keywords((
        'if_statement', 'if_expression', 'while_statement', 'while_expression',
        'for_statement', 'switch_statement', 'switch_expression',
        'break_statement', 'continue_statement', 'return_statement',
        'await_expression', 'let_declaration', 'var_declaration',
    ), drop=('self', 'true', 'false'))

    modifier_kw = {'pub', 'async', 'extern', 'native', 'mut'} & set(
        x.keywords(('function_definition', 'struct_definition', 'use_declaration',
                    'let_declaration', 'var_declaration', 'extend_definition')))

    literals = x.keywords(('boolean',))
    literals |= {'true', 'false'}

    ops = x.operators(('binary_expression', 'unary_expression',
                        'assignment_expression', 'update_expression',
                        'as_expression', 'range_expression',
                        'spread_expression', 'index_expression',
                        'field_expression', 'call_expression'))

    assign_ops = {o for o in ops if o.endswith('=') and o not in ('==', '!=',
                                                                 '<=', '>=')}
    logic_ops = {o for o in ops if o in ('&&', '||', '!')}
    cmp_ops = {o for o in ops if o in ('==', '!=', '<', '>', '<=', '>=')}
    rest_ops = ops - assign_ops - logic_ops - cmp_ops

    declarations = [
        # `fn name(params)` with any modifiers and an optional return type.
        # Indented too: trait members are `    fn draw(self) -> u32;`.
        {'match': r'^(\s*)((?:pub\s+|async\s+|extern\s+|native\s+)*)'
                 r'(fn)(\s+)(%s)' % IDENT,
         'captures': {
             '2': {'name': 'storage.modifier.mu'},
             '3': {'name': 'keyword.declaration.function.mu'},
             '5': {'name': 'entity.name.function.mu'}}},
        # `struct Name {`, `variant Name`, `trait Name`, `type Name`, …
        {'match': r'^\s*((?:pub\s+|async\s+)*)(%s)\s+(%s)'
                 % (alt(decl_kw), IDENT),
         'captures': {
             '2': {'name': 'keyword.declaration.type.mu'},
             '3': {'name': 'entity.name.type.mu'}}},
        # `extend Name: trait Trait {`
        {'match': r'^\s*(extend)\s+(%s)\s*(:\s*trait\s+(%s))?'
                 % (IDENT, IDENT),
         'captures': {
             '1': {'name': 'keyword.declaration.type.mu'},
             '2': {'name': 'entity.name.type.mu'},
             '3': {'name': 'entity.name.type.mu'}}},
        # `const NAME: T = …`
        {'match': r'^\s*(const)\s+(%s)' % IDENT,
         'captures': {
             '1': {'name': 'storage.modifier.mu'},
             '2': {'name': 'variable.other.constant.mu'}}},
        # `let x: T = …` / `var y: T`
        {'match': r'^\s*(let|var)\s+(%s)' % IDENT,
         'captures': {
             '1': {'name': 'keyword.declaration.mu'},
             '2': {'name': 'variable.other.mu'}}},
        # struct members: `name: T,`  and  `fn name(`
        {'match': r'^\s+(%s)\s*(:\s*[^,;]+)?\s*[,;]?\s*$' % IDENT,
         'captures': {
             '1': {'name': 'property.mu'}}},
        # parameters: `name: T`
        {'match': r'^\s+(mut\s+)?(%s)\s*:\s*' % IDENT,
         'captures': {
             '2': {'name': 'variable.parameter.mu'}}},
        # `use std::io;` / `use std::io as x;`
        {'match': r'\b(use)\s+([A-Za-z_][A-Za-z0-9_:.]*)',
         'captures': {
             '1': {'name': 'keyword.control.import.mu'},
             '2': {'name': 'entity.name.namespace.mu'}}},
        # `let x = f(...)` style call: identifier immediately before `(`
        {'match': r'(?<![.\w])(%s)(\s*\()' % IDENT,
         'captures': {
             '1': {'name': 'entity.name.function.call.mu'},
             '2': {'name': 'punctuation.definition.parameters.mu'}}},
        # method call / field access
        {'match': r'(\.)(%s)' % IDENT,
         'captures': {
             '1': {'name': 'punctuation.accessor.mu'},
             '2': {'name': 'variable.other.property.mu'}}},
        # path expression head: `std::io::println`
        {'match': r'\b(%s)(::)' % IDENT,
         'captures': {
             '1': {'name': 'entity.name.namespace.mu'},
             '2': {'name': 'punctuation.accessor.mu'}}},
    ]

    keywords = [
        {'name': 'keyword.declaration.type.mu', 'match': word(decl_kw)},
        {'name': 'keyword.control.mu', 'match': word(control_kw)},
        {'name': 'storage.modifier.mu', 'match': word(modifier_kw)},
        {'name': 'constant.language.mu', 'match': word(literals)},
    ]

    operators = [
        {'name': 'keyword.operator.assignment.mu', 'match': alt(assign_ops)},
        {'name': 'keyword.operator.logical.mu', 'match': alt(logic_ops)},
        {'name': 'keyword.operator.comparison.mu', 'match': alt(cmp_ops)},
        {'name': 'keyword.operator.mu', 'match': alt(rest_ops)},
    ]

    fallbacks = [
        # Capitalised identifiers are conventionally types in Mu.
        ((('identifier'), '@type'),),
    ]
    # Identifier fallbacks go LAST: TextMate takes the first match per span.
    fallbacks = [
        {'match': r'\b([A-Z][A-Za-z0-9_]*)\b', 'name': 'entity.name.type.mu'},
        {'match': r'\b([a-z_][A-Za-z0-9_]*)\b', 'name': 'variable.other.mu'},
    ]

    return {
        'scopeName': 'source.mu',
        'fileTypes': ['mu'],
        'name': 'Mu',
        'patterns': (
            [{'include': f'#{n}'} for n in (
                'comments', 'manifest', 'attributes', 'labels', 'strings',
                'numbers', 'declarations', 'keywords', 'operators',
                'punctuation', 'builtins', 'fallbacks')]
        ),
        'repository': {
            'comments': {'patterns': [
                {'name': 'comment.line.double-slash.mu', 'match': '//.*$'},
                {'name': 'comment.block.mu', 'begin': '/\\*', 'end': '\\*/'},
            ]},
            'manifest': {'patterns': [
                {'name': 'keyword.directive.mu',
                 'match': r'^#!.*$'},
            ]},
            'attributes': {'patterns': [
                {'match': r'(#\[)([A-Za-z_][A-Za-z0-9_]*)',
                 'captures': {
                     '1': {'name': 'punctuation.definition.annotation.mu'},
                     '2': {'name': 'entity.name.function.macro.mu'}}},
            ]},
            'labels': {'patterns': [
                {'match': r'(@)([A-Za-z_][A-Za-z0-9_]*)',
                 'captures': {
                     '1': {'name': 'punctuation.definition.annotation.mu'},
                     '2': {'name': 'label.mu'}}},
            ]},
            'strings': {'patterns': [
                {'name': 'string.quoted.double.mu', 'begin': '"', 'end': '"',
                 'patterns': [
                     {'name': 'constant.character.escape.mu',
                      'match': r'\\.'}]},
                {'name': 'string.quoted.single.mu', 'begin': "'", 'end': "'",
                 'patterns': [
                     {'name': 'constant.character.escape.mu',
                      'match': r'\\.'}]},
            ]},
            'numbers': {'patterns': [
                {'name': 'constant.numeric.integer.hex.mu',
                 'match': r'\b0[xX][0-9a-fA-F][0-9a-fA-F_]*\b'},
                {'name': 'constant.numeric.integer.binary.mu',
                 'match': r'\b0[bB][01][01_]*\b'},
                {'name': 'constant.numeric.float.mu',
                 'match': r'\b[0-9][0-9_]*\.[0-9_]*([eE][+-]?[0-9][0-9_]*)?\b'},
                {'name': 'constant.numeric.integer.mu',
                 'match': r'\b[0-9][0-9_]*\b'},
            ]},
            'declarations': {'patterns': declarations},
            'keywords': {'patterns': [k for k in keywords
                                      if k['match'] != r'\b(?:)\b']},
            'operators': {'patterns': [o for o in operators if o['match']]},
            'punctuation': {'patterns': [
                {'name': 'punctuation.definition.block.mu',
                 'match': '[{}]'},
                {'name': 'punctuation.definition.parameters.mu',
                 'match': '[()]'},
                {'name': 'punctuation.definition.subscript.mu',
                 'match': r'[\[\]]'},
                {'name': 'punctuation.separator.mu', 'match': r'[,;.]'},
                {'name': 'punctuation.definition.colon.mu', 'match': ':'},
                {'name': 'punctuation.definition.annotation.mu',
                 'match': '[#@]'},
            ]},
            'builtins': {'patterns': [
                # scalar and builtin type families
                {'match': r'\b(i8|i16|i32|i64|isize|u8|u16|u32|u64|usize|'
                         r'f32|f64|bool|char|str|string|void)\b',
                 'name': 'storage.type.mu'},
                {'match': r'\b(Array|Map|Set|Queue|Pool|Bitset|Option|Result|'
                         r'Range|Vector|String)\b',
                 'name': 'storage.type.builtin.mu'},
                {'match': r'\b(self)\b', 'name': 'variable.language.self.mu'},
                {'match': r'\b(_)\b', 'name': 'variable.language.mu'},
            ]},
            'fallbacks': {'patterns': fallbacks},
        },
    }


def main():
    grammar = json.loads(GRAMMAR.read_text())
    tm = build(Extractor(grammar))
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(tm, indent=2) + '\n')
    print(f'wrote {OUT}')
    r = tm['repository']
    print(f"  decl kw    {len(r['keywords']['patterns'])} buckets")
    print(f"  operators  {len(r['operators']['patterns'])} buckets")
    print(f"  patterns   {len(tm['patterns'])}")


if __name__ == '__main__':
    sys.exit(main())
