.function_flag_ignorant # args ... {.nop $args}
.function if_only {.if $* {.argfunction}; .else {.nop}}
.function_flag_ignorant c args ... {.combine $args$}
.function_flag_ignorant cc args {.combine $args$}
.function_all_flags a [-*] [-x b ...] c {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }}
.whence_function a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth
.function_all_flags a [-*] [-x ... b] c {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
  .combine $nl
  if_only .test_not_empty $-* {c (-*: ) $-*$ $nl}
  if_only .var_exists -x {c (-x: ) $-x$ $nl}
  if_only .var_exists b {c (b: ) $b$ $nl}
  if_only .var_exists c {c (c: ) $c$ $nl}
}
.whence_function a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth
.whence_function if_only
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
.whence_function recurse
recurse a b c d e f g h i j k l m n o p q r s t u v w x y z
# recurse ((((((((a b))) (c d)) (e f)) (g h)) (i j)) k l) m n (o (p (q (r (s (t (u (v (w (x (y z)))))))))))
recurse (a b) ((c d e) ((f g h) ((i j k) ((l m n) ((o p q) ((r s t) ((u v w) ((x y z)))))))))

.function_all_flags a [-*] x [... y z] {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
  .combine $nl
  .echo (\\$x$: ) $x$1 $nl
  .echo (\\$x$: ) $x$ $nl}
a \( (sec ond) third fourth
#  if (nesting) throw Signal_argm (Argm::Mismatched_parenthesis, in);
# and that line is what thows the earlier parenthesis. i don't know why the
# semicolon causes a mismatched brace error
.nop $A $FIGNORE
