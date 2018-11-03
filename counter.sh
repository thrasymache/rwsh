.global nl (
)
.for &{.internal_functions}$ {.nop
  .if .test_string_unequal $1 .raw_command {
    .function_all_flags &&1 -- args ... {.nop
      .echo signal triggered: $0 \( $args$ \)
      .combine $nl}}
  .else {.function_all_flags &&1 -- args ... {.nop $args}}}
.function_all_flags .prompt {.nop}
.function_all_flags .help {.echo type the strings you want to count $nl}
.binary /usr/bin/printenv
.function_all_flags .autofunction var_name {.nop
  .if .var_exists $var_name {.var_add $var_name 1}
  .else {.global $var_name 1; .nop $$var_name}}
.function_all_flags .function_not_found cmd ... {.nop $cmd}
.nop count even the blank lines
.rm_executable ()
.function_all_flags .shutdown -- args ... {.nop $FIGNORE $args
  .unset nl
  /usr/bin/printenv}
