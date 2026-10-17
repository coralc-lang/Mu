; Mu syntax highlighting

; Identifier fallbacks come first: Zed resolves overlapping captures with the
; last matching pattern winning, so these must precede every specific rule or
; they would flatten function, trait, struct and field names to one colour.
((identifier) @variable
 (#lua-match? @variable "^[a-z_]"))

((identifier) @type
 (#lua-match? @type "^[A-Z]"))
;
; Ordering matters: tree-sitter takes the first pattern that matches a node, so
; every specific capture has to come before the generic identifier fallbacks
; at the bottom. With the fallbacks first, every function, trait, struct and
; field name collapsed onto plain @type or @variable and the file read as one
; flat colour.

; ---------- comments / manifest / attributes ----------
(comment) @comment
(manifest) @keyword.directive
(attribute) @attribute
(label) @label

; ---------- literals ----------
(number) @number
(boolean) @constant.builtin
(char) @character
(string) @string
(map_entry key: (string) @property)

; ---------- keywords ----------
"fn" @keyword.function
"struct" @keyword.declaration.type
"variant" @keyword.declaration.type
"trait" @keyword.declaration.type
"extend" @keyword.declaration.type
"type" @keyword.declaration.type

"pub" @keyword.modifier
"async" @keyword.modifier
"extern" @keyword.modifier
"native" @keyword.modifier
"mut" @keyword.modifier
"const" @keyword.modifier

"use" @keyword.control.import
"as" @keyword.control.import

"let" @keyword.declaration
"var" @keyword.declaration

"if" @keyword.control.conditional
"else" @keyword.control.conditional
"switch" @keyword.control.conditional

"while" @keyword.control.loop
"for" @keyword.control.loop
"in" @keyword.control.loop
"break" @keyword.control.loop
"continue" @keyword.control.loop

"return" @keyword.control.flow
"await" @keyword.control.flow

; ---------- declared names ----------
; These must precede the identifier fallbacks.
(function_definition name: (identifier) @function)
(trait_member (identifier) @function.method)

(struct_definition name: (identifier) @type)
(variant_definition name: (identifier) @type)
(trait_definition name: (identifier) @type)
(type_definition name: (identifier) @type)
(const_definition name: (identifier) @constant)

; `extend Name: trait Trait { … }`
(extend_definition name: (identifier) @type)
(extend_definition trait: (identifier) @type)

(struct_member (identifier) @property)
(field_initializer name: (identifier) @property)

(variant_case (identifier) @constructor)
(variant_pattern (identifier) @constructor)

(parameter (identifier) @variable.parameter)
; `self` in a parameter list is the anonymous token 'self' (grammar.js picks
; `$.parameter` or the literal), so it needs a direct string capture.
"self" @variable.builtin
(let_declaration pattern: (identifier) @variable)
(var_declaration pattern: (identifier) @variable)
(rest_pattern (identifier) @variable)
(slice_pattern (identifier) @variable)

; `self` is a keyword-like token in expression position and a plain
; identifier in a parameter list, so it needs both.
(self_expression) @variable.builtin
(wildcard_pattern) @variable.builtin

; ---------- calls and member access ----------
(call_expression function: (identifier) @function.call)
(call_expression function: (field_expression field: (identifier) @method.call))
(path_expression (identifier) @function.call)

; ---------- types ----------
(path_type) @type
(generic_type) @type
(dynamic_type) @type
(tuple_type) @type
(slice_type) @type
(array_type) @type

; builtin containers and scalar families
((identifier) @type.builtin
 (#any-of? @type.builtin
   "Array" "Map" "Set" "Queue" "Pool" "Bitset" "Option" "Result" "Range"
   "i8" "i16" "i32" "i64" "isize" "u8" "u16" "u32" "u64" "usize"
   "f32" "f64" "bool" "char" "str" "string"))

; Capitalised identifiers are conventionally types in Mu.
((identifier) @type
 (#lua-match? @type "^[A-Z]"))

; ---------- operators ----------
[
  "=" "+=" "-=" "*=" "/=" "%=" "&=" "|=" "^=" "<<=" ">>="
] @operator

[
  "+" "-" "*" "/" "%" "&" "|" "^" "~" "<<" ">>"
  "==" "!=" "<" "<=" ">" ">=" "&&" "||" "!"
] @operator

[
  "?" "..." ".." "..=" "::"
] @operator

; ---------- punctuation ----------
[
  "(" ")" "[" "]" "{" "}"
] @punctuation.bracket

[
  "." "," ";" ":"
] @punctuation.delimiter
