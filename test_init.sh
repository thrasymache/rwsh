.global nl (
)
.try_catch_recursive rwsh.file_open_failure {
  .source /non-existent/file/to/test/failure}
.function_all_flags rwsh.raw_command -- cmd {.echo $cmd; .echo $nl}
.for ${.echo rwsh.sigterm rwsh.multiple_argfunctions}$ {.nop
  .function_all_flags $1 {.echo signal triggered: $0 \( \); .combine $nl; .return -1}}
.global last_command_return -10
.global A \
.global N \
.nop $N

.nop .function_all_flags rwsh.run_logic -- cmd [args ...] {
  .return $last_command_return
  .signal_handler &{.internal_functions}$ {&&* {rwsh.argfunction}}
  rwsh.mapped_argfunction {&&* {rwsh.argfunction}}
  &&* {rwsh.argfunction}
  .set last_command_return $?
  .echo $nl
  .echo $last_command_return
  .echo $nl}
.function_all_flags rwsh.before_command -- args ... {
  .nop $args
  .return $last_command_return}
.function_all_flags rwsh.after_command -- args ... {
  .nop $args
  .set last_command_return $?
  .combine $nl $last_command_return $nl}
.function_all_flags rwsh.prompt {.echo \$}
.function_all_flags rwsh.vars {.internal_vars}
.source /etc/rwshrc-basic
