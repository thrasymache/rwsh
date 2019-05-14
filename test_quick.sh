.function a [-*] [-x b ...] c {
  forj &&{.list_locals}$ j {.combine $j \( $$j \) \ }; .echo $nl}
.nop $A $FIGNORE
.whence_function a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth
.function a [-*] [-x ... b] c {
  forj &&{.list_locals}$ {.combine $j \( $$j \) \ }
  .combine $nl
  # if_only .test_not_empty $-* {c (-*: ) $-*$ $nl}
  # if_only .var_exists -x {c (-x: ) $-x$ $nl}
  # if_only .var_exists b {c (b: ) $b$ $nl}
  # if_only .var_exists c {c (c: ) $c$ $nl}
}
.whence_function a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth

rpn 1
rpn +
rpn 1 1
rpn 1 +
rpn + 1
rpn + +
rpn 1 1 1
rpn 1 1 +
rpn 1 + 1
rpn 1 + +
rpn + 1 1
rpn + 1 +
rpn + + 1
rpn + + +
rpn 1 1 1 1
rpn 1 1 1 +
rpn 1 1 + 1
rpn 1 1 + +
rpn 1 + 1 1
rpn 1 + 1 +
rpn 1 + + 1
rpn 1 + + +
rpn + 1 1 1
rpn + 1 1 +
rpn + 1 + 1
rpn + 1 + +
rpn + + 1 1
rpn + + 1 +
rpn + + + 1
rpn + + + +
rpn 1 1 1 1 1
rpn 1 1 1 1 +
rpn 1 1 1 + 1
rpn 1 1 1 + +
rpn 1 1 + 1 1
rpn 1 1 + 1 +
rpn 1 1 + + 1
rpn 1 1 + + +
rpn 1 + 1 1 1
rpn 1 + 1 1 +
rpn 1 + 1 + 1
rpn 1 + 1 + +
rpn 1 + + 1 1
rpn 1 + + 1 +
rpn 1 + + + 1
rpn 1 + + + +
rpn + 1 1 1 1
rpn + 1 1 1 +
rpn + 1 1 + 1
rpn + 1 1 + +
rpn + 1 + 1 1
rpn + 1 + 1 +
rpn + 1 + + 1
rpn + 1 + + +
rpn + + 1 1 1
rpn + + 1 1 +
rpn + + 1 + 1
rpn + + 1 + +
rpn + + + 1 1
rpn + + + 1 +
rpn + + + + 1
rpn + + + + +
rpn 3 2 *
rpn 1 2 3 4 5 6 7 8 9 10 + + + + + + + + +
rpn 1 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 +
rpn 1 2 + 3 4 + + 5 6 + 7 8 + + + 9 10 + +
rpn 10 26 + 9 10 + +
rpn 10 26 + 9 10 +
arpn 3 2 *
arpn 1 2 3 4 5 6 7 8 9 10 + + + + + + + + +
arpn 1 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 +
arpn 1 2 + 3 4 + + 5 6 + 7 8 + + + 9 10 + +
arpn 10 26 + 9 10 + +
arpn 10 26 + 9 10 +
arpn 1 2 3 + +
arpn 2 3 + +
echo ${.total_execution_time arpn}
echo ${.total_execution_time rpn}
rpn 10 26 + 19 +
.scope 1 2 3 4 5 (args ...) {
  .list_locals; echo \  $args$}
.scope 1 2 3 4 5 (args ...) {
  .reinterpret $args$ (before args ... after)}
.scope ([before after]) {
  .scope 1 2 3 4 5 (args ...) {
    .reinterpret $args$ (before args ... after)
    .list_locals; echo $nl $args$}}
.scope ([before after]) {
  .scope 1 2 3 4 5 (args ...) {
    .reinterpret $args$ (before args ... after)
    .list_locals; echo () $after $before and now $args$}}
.scope foo [bar] {
  echo i have $bar$
  .reinterpret [bar]
  echo i have $bar$}
