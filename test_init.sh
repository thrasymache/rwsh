.global nl (
)
.for &{.internal_functions}$ {.function $1 {.echo signal triggered: $0 \( $* \) $nl; .return -1}}
.function rwsh.raw_command {.echo $1; .echo $nl}
.global last_command_return -10
.global A \
.global N \
.nop $N

.nop tests multi-line commands within a sourced script
.nop .function rwsh.run_logic {
  .return $last_command_return
  .signal_handler &{.internal_functions}$ {&&* {rwsh.argfunction}}
  rwsh.mapped_argfunction {&&* {rwsh.argfunction}}
  &&* {rwsh.argfunction}
  .set last_command_return $?
  .echo $nl
  .echo $last_command_return
  .echo $nl}
.function rwsh.before_command {.return $last_command_return}
.function rwsh.after_command {
  .set last_command_return $?
  .echo $nl; .echo $last_command_return; .echo $nl}
.function rwsh.prompt {.echo \$}
.function rwsh.shutdown {.echo $nl; .echo now terminating normally; .echo $nl}
.function rwsh.vars {.internal_vars}
