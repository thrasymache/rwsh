.try_catch_recursive .file_open_failure {
  .source /non-existent/file/to/test/failure}
.function_all_flags .raw_command -- cmd {.echo $cmd; .echo (
)}
.global A \

.nop .function_all_flags .run_logic -- cmd [args ...] {
  .signal_handler &{.internal_functions}$ {&&* {.argfunction}}
  .mapped_argfunction {&&* {.argfunction}}
  &&* {.argfunction}}
.function_all_flags .before_command -- args ... [.{argfunction}] {.nop $args}
.function_all_flags .after_command -- args ... [.{argfunction}] {.nop $args}
.function_all_flags .prompt {.echo \$}
.function_all_flags .vars {.internal_vars}
.source /etc/rwshrc-basic
