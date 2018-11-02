.function_all_flags a [-*] [-x b ...] c {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }}
.nop $A $FIGNORE
.whence_function a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth
.function_all_flags a [-*] [-x ... b] c {
  .for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
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
