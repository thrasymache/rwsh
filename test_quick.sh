.function_flag_ignorant # args ... {.nop $args}
.function if_only {.if $* {rwsh.argfunction}; .else {.nop}}
.function_flag_ignorant c args ... {.combine $args$}
.function_flag_ignorant cc args {.combine $args$}
.function_some_flags a [-x b ...] c {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }}
.which_executable a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth
.function_some_flags a [-x ... b] c {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
  .combine $nl
  if_only .test_not_empty $-* {c (-*: ) $-*$ $nl}
  if_only .var_exists -x {c (-x: ) $-x$ $nl}
  if_only .var_exists b {c (b: ) $b$ $nl}
  if_only .var_exists c {c (c: ) $c$ $nl}
}
.which_executable a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth
.which_executable if_only
.combine &{c (sec ond)} x $nl x
.combine &{c (sec ond)}$ x $nl x
.combine &{cc (sec ond)} x $nl x
.combine &{cc (sec ond)}$ x $nl x
.combine &{.combine &{.combine ((sec ond))}}$ x $nl x
.combine &{.combine &{.combine ((sec ond))}$}$ x $nl x
.global nest 0
.set MAX_NESTING 90
.function_flag_ignorant rx lhs rhs ... {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
  .combine $nl
  recurse $lhs $rhs}
.function_flag_ignorant recurse lhs ... rhs {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) $nl}
  .combine $nl
  .var_add nest 1
  .if .test_less $nest 5 {recurse $lhs $rhs$}
  .else_if .test_less $nest 10 {recurse $lhs$ $rhs}
  .else_if .test_less $nest 15 {rx $lhs$ $rhs}
  .else_if .test_less $nest 20 {rx $lhs$ $rhs}
  .else {.nop}
  .var_subtract nest 1}
.which_executable recurse
recurse a b c d e f g h i j k l m n o p q r s t u v w x y z
# recurse ((((((((a b))) (c d)) (e f)) (g h)) (i j)) k l) m n (o (p (q (r (s (t (u (v (w (x (y z)))))))))))
recurse (a b) ((c d e) ((f g h) ((i j k) ((l m n) ((o p q) ((r s t) ((u v w) ((x y z)))))))))

