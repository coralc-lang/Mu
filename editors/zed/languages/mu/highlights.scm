; Mu syntax highlighting

; ---------- comments ----------
(comment) @comment

; ---------- manifests and attributes ----------
(manifest) @keyword.directive

(attribute) @attribute

; ---------- declarations ----------
"fn" @keyword.function
"struct" @keyword.type
"variant" @keyword.type
"trait" @keyword.type
"extend" @keyword.type
"type" @keyword.type
"const" @keyword

"pub" @keyword.modifier
"async" @keyword.modifier
"extern" @keyword.modifier
"native" @keyword.modifier
"mut" @keyword.modifier

"use" @keyword.import
"as" @keyword.import

(function_definition name: (identifier) @function)
(trait_member (identifier) @function)
(struct_member (identifier) @property)
(field_initializer name: (identifier) @property)
(variant_pattern (identifier) @property)

(parameter (identifier) @variable.parameter)
(variant_pattern (identifier) @variable)
(variant_case (identifier) @constructor)
(variant_pattern (identifier) @constructor)

(struct_definition name: (identifier) @type)
(variant_definition name: (identifier) @type)
(trait_definition name: (identifier) @type)
(extend_definition name: (identifier) @type)
(type_definition name: (identifier) @type)

; ---------- statements ----------
"var" @keyword
"let" @keyword
"if" @keyword.conditional
"else" @keyword.conditional
"while" @keyword.repeat
"for" @keyword.repeat
"in" @keyword.repeat
"switch" @keyword.conditional
"return" @keyword.return
"break" @keyword
"continue" @keyword
"await" @keyword

(label) @label

; ---------- operators ----------
[
  "=" "+=" "-=" "*=" "/=" "%=" "&=" "|=" "^=" "<<=" ">>="
] @operator

[
  "+" "-" "*" "/" "%" "&" "|" "^" "~" "<<" ">>"
  "==" "!=" "<" "<=" ">" ">=" "&&" "||" "!"
] @operator

"as" @operator
"?" @operator
"..." @operator
".." @operator
"..=" @operator
"::" @operator
"." @punctuation.delimiter
":" @punctuation.delimiter
";" @punctuation.delimiter
"," @punctuation.delimiter

; ---------- literals ----------
(number) @number
(boolean) @boolean
(char) @character
(string) @string
(map_entry key: (string) @string)

(self_expression) @variable.builtin
(wildcard_pattern) @variable.builtin

; ---------- types ----------
[
  (identifier) @type
  (path_type) @type
  (generic_type) @type
]
((identifier) @type
 (#lua-match? @type "^[A-Z]"))

; `_type_atom` is a hidden rule; it expands to `identifier`, so
; identifiers in type position are styled as types below.
((identifier) @variable
 (#lua-match? @variable "^[a-z_]"))

; builtin containers and scalar families keep their own color
((identifier) @type.builtin
 (#any-of? @type.builtin
   "Array" "Map" "Set" "Queue" "Pool" "Bitset" "Option" "Result" "Range"
   "i8" "i16" "i32" "i64" "isize" "u8" "u16" "u32" "u64" "usize"
   "f32" "f64" "bool" "char" "str" "string"))

; ---------- calls ----------
(call_expression function: (identifier) @function)
(call_expression function: (field_expression field: (identifier) @function))
