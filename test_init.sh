.try_catch_recursive .file_open_failure {
  .source /non-existent/file/to/test/failure}
.function_all_flags .raw_command -- cmd {.echo $cmd; .echo (
)}
.global last_command_return -10
.global A \

.nop .function_all_flags .run_logic -- cmd [args ...] {
  .return $last_command_return
  .signal_handler &{.internal_functions}$ {&&* {.argfunction}}
  .mapped_argfunction {&&* {.argfunction}}
  &&* {.argfunction}
  .set last_command_return $?
  .echo $nl
  .echo $last_command_return
  .echo $nl}
.function_all_flags .before_command -- args ... {
  .nop $args
  .return $last_command_return}
.function_all_flags .after_command -- args ... {
  .nop $args
  .set last_command_return $?
  if_only .test_string_unequal $last_command_return 0 {
    .combine $last_command_return $nl}}
.function_all_flags .prompt {.echo \$}
.function_all_flags .vars {.internal_vars}
.source /etc/rwshrc-basic
