.set MAX_NESTING 7
.global nl (
)
.function rwsh.raw_command {.echo $1; .echo $nl}
.function # {.nop}
.global last_command_return -10
.global A \
.global N \
.function rwsh.before_command {.return $last_command_return}

# tests multi-line commands within a sourced script
.function rwsh.after_command {
  .set last_command_return $?
  .echo $nl; .echo $last_command_return; .echo $nl}
.function rwsh.prompt {.echo \$}
.function rwsh.file_open_failure
.function rwsh.shutdown {.echo $nl; .echo now terminating normally; .echo $nl}
.function rwsh.vars {.internal_vars}
