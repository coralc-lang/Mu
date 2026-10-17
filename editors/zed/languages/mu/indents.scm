; Mu indentation: blocks and call/list bodies open a new line, closers dedent.

[
  (block)
  (argument_list)
  (parameter_list)
  (array_expression)
  (map_expression)
  (struct_expression)
  (tuple_expression)
  (tuple_type)
  (tuple_pattern)
  (switch_statement)
  (switch_expression)
  (struct_definition)
  (trait_definition)
  (extend_definition)
  (variant_definition)
] @indent

[
  ")"
  "]"
  "}"
] @outdent
