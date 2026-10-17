; Mu symbol outline

(source_file
  (function_definition
    name: (identifier) @name
    (#icon? "function"))) @item

(source_file
  (struct_definition
    name: (identifier) @name
    (#icon? "type"))) @item

(source_file
  (variant_definition
    name: (identifier) @name
    (#icon? "enum"))) @item

(source_file
  (trait_definition
    name: (identifier) @name
    (#icon? "interface"))) @item

(source_file
  (extend_definition
    name: (identifier) @name
    (#icon? "extension"))) @item

(source_file
  (type_definition
    name: (identifier) @name
    (#icon? "type"))) @item

(source_file
  (const_definition
    name: (identifier) @name
    (#icon? "constant"))) @item

(source_file
  (use_declaration) @context
  (function_definition
    name: (identifier) @name
    (#icon? "function"))) @item

(struct_definition
  (struct_member
    name: (identifier) @name
    (#icon? "field"))) @item

(variant_definition
  (variant_case
    (identifier) @name
    (#icon? "variant"))) @item

(trait_definition
  (trait_member
    name: (identifier) @name
    (#icon? "function"))) @item

(extend_definition
  (function_definition
    name: (identifier) @name
    (#icon? "function"))) @item

(struct_definition
  (function_definition
    name: (identifier) @name
    (#icon? "method"))) @item
