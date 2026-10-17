module.exports = grammar({
  name: 'mu',

  // control-flow forms are both statements and expressions in mu; tree-sitter
  // resolves the resulting ambiguities with declared conflicts (glr).
  conflicts: $ => [
    [$.if_statement, $.if_expression],
    [$.switch_statement, $.switch_expression],
    [$.while_statement, $.while_expression],
  ],

  extras: $ => [
    $.comment,
    /\s/,
  ],

  inline: $ => [
    $._statement,
    $._expression,
    $._pattern,
  ],

  word: $ => $.identifier,

  rules: {
    // ---------------- top level ----------------

    source_file: $ => repeat($._item),

    manifest: $ => seq('#!', /.*/),

    _item: $ => choice(
      $.manifest,
      $.use_declaration,
      $.function_definition,
      $.struct_definition,
      $.variant_definition,
      $.trait_definition,
      $.extend_definition,
      $.type_definition,
      $.const_definition,
    ),

    // ---------- use ----------

    use_declaration: $ => seq(
      'use',
      field('path', $.use_path),
      optional(seq('as', $.identifier)),
      ';',
    ),

    use_path: $ => seq(
      $.identifier,
      repeat(seq('::', $.identifier)),
      optional(seq('::', '{', commaSep1($.use_spec), '}')),
    ),

    use_spec: $ => seq(
      $.identifier,
      optional(seq('as', $.identifier)),
    ),

    // ---------- attributes / modifiers ----------

    // mu writes attributes as `#[[name, arg, ...]]`
    attribute: $ => choice(
      seq('#[[', commaSep1(choice($.identifier, $.number, $.string, $.boolean)), ']]'),
      seq('@', $.identifier),
    ),

    // label: `@outer:` — the @ marks it so it can never be an expression
    label: $ => seq('@', $.identifier, ':'),

    // ---------- functions ----------

    function_definition: $ => prec.right(seq(
      repeat($.attribute),
      repeat(choice('pub', 'async', 'extern', 'native')),
      'fn',
      field('name', $.identifier),
      field('parameters', $.parameter_list),
      optional(field('return_type', $._type)),
      field('body', $.block),
    )),

    parameter_list: $ => seq(
      '(',
      optional(commaSep1(choice($.parameter, 'self'))),
      ')',
    ),

    parameter: $ => seq(
      optional('mut'),
      $.identifier,
      ':',
      $._type,
    ),

    // ---------- aggregates ----------

    struct_definition: $ => seq(
      repeat($.attribute),
      repeat(choice('pub', 'async', 'extern', 'native')),
      'struct',
      field('name', $.identifier),
      '{',
      repeat(seq(choice($.struct_member, $.function_definition), ';')),
      '}',
      optional(';'),
    ),

    struct_member: $ => seq(
      repeat($.attribute),
      optional('pub'),
      $.identifier,
      ':',
      $._type,
    ),

    variant_definition: $ => seq(
      repeat($.attribute),
      repeat(choice('pub', 'async', 'extern', 'native')),
      'variant',
      field('name', $.identifier),
      '{',
      optional(commaSep1($.variant_case)),
      optional(','),
      '}',
      optional(';'),
    ),

    variant_case: $ => seq(
      repeat($.attribute),
      $.identifier,
      optional(seq('(', optional(commaSep1($._type)), ')')),
    ),

    trait_definition: $ => seq(
      repeat($.attribute),
      repeat(choice('pub', 'async', 'extern', 'native')),
      'trait',
      field('name', $.identifier),
      '{',
      repeat(seq($.trait_member, ';')),
      '}',
      optional(';'),
    ),

    trait_member: $ => seq(
      repeat($.attribute),
      optional('pub'),
      'fn',
      $.identifier,
      $.parameter_list,
      optional(field('return_type', $._type)),
      optional($.block),
    ),

    extend_definition: $ => seq(
      repeat(choice('pub', 'async', 'extern', 'native')),
      'extend',
      field('name', $.identifier),
      optional(seq(':', 'trait', field('trait', $.identifier))),
      '{',
      repeat(seq(choice($.function_definition, $.type_definition, $.const_definition), ';')),
      '}',
      optional(';'),
    ),

    // ---------- type aliases and constants ----------

    type_definition: $ => seq(
      repeat($.attribute),
      optional('pub'),
      'type',
      field('name', $.identifier),
      '=',
      $._type,
      ';',
    ),

    const_definition: $ => seq(
      repeat($.attribute),
      optional('pub'),
      'const',
      field('name', $.identifier),
      optional(seq(':', $._type)),
      '=',
      field('value', $._expression),
      ';',
    ),

    // ---------- types ----------

    _type: $ => choice(
      $._type_atom,
      $.tuple_type,
      $.slice_type,
      $.array_type,
      $.generic_type,
      $.path_type,
    ),

    _type_atom: $ => choice(
      $.identifier,
      'self',
    ),

    // i32[3], and `[T; N]` style
    array_type: $ => prec.left(6, seq(
      $._type_atom,
      '[',
      field('length', $._expression),
      ']',
    )),

    // i32[] — borrow, non-owning
    slice_type: $ => prec.left(6, seq(
      $._type_atom,
      '[',
      ']',
    )),

    // (bool, u32), and (T,) for one-tuples
    tuple_type: $ => seq(
      '(',
      commaSep1($._type),
      optional(','),
      ')',
    ),

    // Array<T>, Map<K, V>, Set<T>, Queue<T>, Pool<T>, Option<T>, Result<T, E>
    generic_type: $ => prec.right(6, seq(
      $._type_atom,
      '<',
      commaSep1($._type),
      optional(','),
      '>',
    )),

    path_type: $ => prec.right(6, seq(
      $._type_atom,
      repeat1(seq('::', $._type_atom)),
    )),

    // ---------- statements ----------

    block: $ => seq(
      '{',
      repeat($._statement),
      '}',
    ),

    _statement: $ => choice(
      $.let_declaration,
      $.var_declaration,
      $.if_statement,
      $.while_statement,
      $.for_statement,
      $.switch_statement,
      $.return_statement,
      $.break_statement,
      $.continue_statement,
      $.expression_statement,
    ),

    let_declaration: $ => seq(
      'let',
      optional('mut'),
      field('pattern', $._pattern),
      optional(seq(':', $._type)),
      '=',
      field('value', $._expression),
      ';',
    ),

    var_declaration: $ => seq(
      'var',
      optional('mut'),
      field('pattern', $._pattern),
      optional(seq(':', $._type)),
      optional(seq('=', field('value', $._expression))),
      ';',
    ),

    if_statement: $ => seq(
      'if',
      field('condition', $._expression),
      field('consequence', $.block),
      optional(seq('else', field('alternative', choice($.block, $.if_statement)))),
    ),

    while_statement: $ => seq(
      optional($.label),
      'while',
      field('condition', $._expression),
      field('body', $.block),
    ),

    for_statement: $ => seq(
      optional($.label),
      'for',
      field('pattern', $._pattern),
      'in',
      field('iterable', $._expression),
      field('body', $.block),
    ),

    switch_statement: $ => seq(
      optional($.label),
      'switch',
      '(',
      field('value', $._expression),
      ')',
      '{',
      optional(commaSep1($.switch_prong)),
      optional(','),
      '}',
    ),

    switch_prong: $ => seq(
      choice(
        commaSep1($._pattern),
        'else',
      ),
      optional(seq('if', field('guard', $._expression))),
      '=>',
      field('body', choice($._expression, $.block)),
    ),

    return_statement: $ => seq(
      'return',
      optional($._expression),
      ';',
    ),

    break_statement: $ => seq(
      'break',
      optional($.identifier),
      ';',
    ),

    continue_statement: $ => seq(
      'continue',
      optional($.identifier),
      ';',
    ),

    expression_statement: $ => seq(
      $._expression,
      ';',
    ),

    // ---------- patterns ----------
    // a bare identifier is a binding; a variant case must carry `(...)` or a
    // `::` path so the two are distinguishable without a symbol table.

    _pattern: $ => choice(
      $.wildcard_pattern,
      $.tuple_pattern,
      $.variant_pattern,
      $.slice_pattern,
      $.literal_pattern,
      $.range_pattern,
      $.identifier,
    ),

    wildcard_pattern: $ => '_',

    tuple_pattern: $ => seq(
      '(',
      commaSep1($._pattern),
      optional(','),
      ')',
    ),

    variant_pattern: $ => prec.left(seq(
      $.identifier,
      choice(
        seq('::', $.identifier, optional(seq('(', optional(commaSep1($._pattern)), ')'))),
        seq('(', optional(commaSep1($._pattern)), ')'),
      ),
    )),

    slice_pattern: $ => seq(
      '[',
      optional(commaSep1(choice($._pattern, $.rest_pattern))),
      optional(','),
      ']',
    ),

    rest_pattern: $ => seq('...', $.identifier),

    literal_pattern: $ => choice(
      $.number,
      $.string,
      $.char,
      $.boolean,
      $.unary_expression,
    ),

    range_pattern: $ => prec.left(seq(
      $._pattern,
      choice('..', '..='),
      $._pattern,
    )),

    // ---------- expressions ----------

    _expression: $ => choice(
      $.identifier,
      $.number,
      $.string,
      $.char,
      $.boolean,
      $.self_expression,
      $.unary_expression,
      $.binary_expression,
      $.update_expression,
      $.assignment_expression,
      $.call_expression,
      $.field_expression,
      $.index_expression,
      $.array_expression,
      $.map_expression,
      $.range_expression,
      $.tuple_expression,
      $.struct_expression,
      $.if_expression,
      $.switch_expression,
      $.while_expression,
      $.await_expression,
      $.question_expression,
      $.parenthesized_expression,
      $.as_expression,
    ),

    self_expression: $ => 'self',

    unary_expression: $ => prec(16, seq(
      choice('-', '!', '~'),
      $._expression,
    )),

    binary_expression: $ => {
      const table = [
        ['||'],
        ['&&'],
        ['|'],
        ['^'],
        ['&'],
        ['==', '!='],
        ['<', '<=', '>', '>='],
        ['<<', '>>'],
        ['+', '-'],
        ['*', '/', '%'],
      ];

      return choice(...table.map((operators, precedence) =>
        prec.left(6 + precedence, seq(
          field('left', $._expression),
          field('operator', choice(...operators)),
          field('right', $._expression),
        )),
      ));
    },

    update_expression: $ => prec.left(2, seq(
      field('left', $._expression),
      field('operator', choice('+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', '<<=', '>>=')),
      field('right', $._expression),
    )),

    assignment_expression: $ => prec.right(1, seq(
      field('left', $._expression),
      '=',
      field('right', $._expression),
    )),

    // f(x), x.f(y), T::new(x)
    call_expression: $ => prec(17, seq(
      field('function', $._expression),
      field('arguments', $.argument_list),
    )),

    argument_list: $ => seq(
      '(',
      optional(commaSep1($._expression)),
      optional(','),
      ')',
    ),

    field_expression: $ => prec(18, seq(
      field('object', $._expression),
      '.',
      field('field', $.identifier),
    )),

    index_expression: $ => prec(18, seq(
      field('object', $._expression),
      '[',
      field('index', $._expression),
      ']',
    )),

    as_expression: $ => prec.right(4, seq(
      $._expression,
      'as',
      $._type,
    )),

    await_expression: $ => prec(16, seq(
      'await',
      $._expression,
    )),

    question_expression: $ => prec(4, seq(
      $._expression,
      '?',
    )),

    parenthesized_expression: $ => seq(
      '(',
      $._expression,
      ')',
    ),

    // [1, 2, 3] and spreads
    array_expression: $ => seq(
      '[',
      optional(commaSep1(choice($._expression, $.spread_expression))),
      optional(','),
      ']',
    ),

    spread_expression: $ => seq('...', $._expression),

    // ["hp": 100, ...] — a leading `:` selects map mode, so a map's first
    // element is always a key/value pair; `[...b]` on its own is an array spread
    map_expression: $ => seq(
      '[',
      $.map_entry,
      repeat(seq(',', choice($.map_entry, $.spread_expression))),
      optional(','),
      ']',
    ),

    map_entry: $ => seq(
      field('key', $._expression),
      ':',
      field('value', $._expression),
    ),

    range_expression: $ => prec.left(3, seq(
      $._expression,
      choice('..', '..='),
      $._expression,
    )),

    tuple_expression: $ => seq(
      '(',
      $._expression,
      ',',
      optional(seq(commaSep1($._expression), optional(','))),
      ')',
    ),

    // Name { field: value, ... }
    struct_expression: $ => prec.right(1, seq(
      field('type', choice($.identifier, $.path_expression)),
      '{',
      optional(commaSep1($.field_initializer)),
      optional(','),
      '}',
    )),

    field_initializer: $ => seq(
      field('name', $.identifier),
      optional(seq(':', field('value', $._expression))),
    ),

    path_expression: $ => seq(
      $.identifier,
      repeat1(seq('::', $.identifier)),
    ),

    if_expression: $ => seq(
      'if',
      field('condition', $._expression),
      field('consequence', $.block),
      optional(seq('else', field('alternative', choice($.block, $.if_expression)))),
    ),

    switch_expression: $ => seq(
      'switch',
      '(',
      field('value', $._expression),
      ')',
      '{',
      optional(commaSep1($.switch_prong)),
      optional(','),
      '}',
    ),

    while_expression: $ => seq(
      'while',
      field('condition', $._expression),
      field('body', $.block),
    ),

    // ---------- terminals ----------

    identifier: $ => /[A-Za-z_][A-Za-z0-9_]*/,

    boolean: $ => choice('true', 'false'),

    // 1, 1_000, 0xFF, 42i8, 1u32, 3.14, 3.14f32
    number: $ => token(seq(
      choice(
        /0[xX][0-9a-fA-F_]+/,
        /0[bB][01_]+/,
        /[0-9][0-9_]*/,
      ),
      optional(seq('.', /[0-9][0-9_]*/)),
      optional(choice(
        /[eE][+-]?[0-9]+/,
      )),
      optional(choice('i8', 'i16', 'i32', 'i64', 'isize', 'u8', 'u16', 'u32', 'u64', 'usize', 'f32', 'f64')),
    )),

    char: $ => token(seq(
      '\'',
      choice(/[^\\'\n]/, /\\./u, /\\u\{[0-9a-fA-F]{1,6}\}/),
      '\'',
    )),

    string: $ => token(seq(
      '"',
      repeat(choice(
        /[^"\\\n]+/,
        /\\./u,
        /\\u\{[0-9a-fA-F]{1,6}\}/,
      )),
      optional('"'),
    )),

    comment: $ => token(choice(
      seq('//', /.*/),
      seq('/*', /[^*]*\*+([^/*][^*]*\*+)*/, '/'),
    )),
  },
});

function commaSep1(rule) {
  return seq(rule, repeat(seq(',', rule)));
}
