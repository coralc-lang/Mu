; Bracket matching for Mu. Strings and comments are excluded so a quote inside
; a string does not open a bracket.

("{") @open
("}") @close
("[") @open
("]") @close
("(") @open
(")") @close

((string) @open
  (#set! rainbow.exclude))
