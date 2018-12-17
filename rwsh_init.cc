// the registering of builtins
// Copyright Samuel Newbold 2005-2018

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "arg_script.h"
#include "argm.h"
#include "rwsh_init.h"
#include "builtin.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

void internal_init(Error_list& exceptions) {
  Argm empty_prototype(Variable_map::global_map,
                       default_input, default_output, default_error);
  Argm any_args(Variable_map::global_map,
                default_input, default_output, default_error);
  any_args.push_back("--");
  any_args.push_back("[args");
  any_args.push_back("...]");
  executable_map.set(new Builtin(".argc", b_argc));
  executable_map.set(new Builtin(".binary", b_binary));
  executable_map.set(new Builtin(".cd", b_cd));
  executable_map.set(new Builtin(".collect_errors_except",
                                 b_collect_errors_except));
  executable_map.set(new Builtin(".collect_errors_only",
                                 b_collect_errors_only));
  executable_map.set(new Builtin(".combine", b_combine));
  executable_map.set(new Builtin(".enable_readline", b_enable_readline));
  executable_map.set(new Builtin(".echo", b_echo));
  executable_map.set(new Builtin(".error", b_error));
  executable_map.set(new Builtin(".disable_readline", b_disable_readline));
  executable_map.set(new Builtin(".else", b_else));
  executable_map.set(new Builtin(".else_if", b_else_if));
  executable_map.set(new Builtin(".else_if_not", b_else_if_not));
  executable_map.set(new Builtin(".exec", b_exec));
  executable_map.set(new Builtin(".execution_count",
                                 b_execution_count));
  executable_map.set(new Builtin(".exit", b_exit));
  executable_map.set(new Builtin(".fallback_handler", b_fallback_handler));
  executable_map.set(new Builtin(".get_fallback_message",
                                 b_get_fallback_message));
  executable_map.set(new Builtin(".get_max_collectible_exceptions",
                                 b_get_max_collectible_exceptions));
  executable_map.set(new Builtin(".get_max_extra_exceptions",
                                 b_get_max_extra_exceptions));
  executable_map.set(new Builtin(".get_max_nesting", b_get_max_nesting));
  executable_map.set(new Builtin(".getpid", b_getpid));
  executable_map.set(new Builtin(".getppid", b_getppid));
  executable_map.set(new Builtin(".for", b_for));
  executable_map.set(new Builtin(".for_each_line", b_for_each_line));
  executable_map.set(new Builtin(".fork", b_fork));
  executable_map.set(new Builtin(".function", b_function));
  executable_map.set(new Builtin(".function_all_flags",
                                 b_function_all_flags));
  executable_map.set(new Builtin(".global", b_global));
  executable_map.set(new Builtin(".if", b_if));
  executable_map.set(new Function(".init",
      any_args.begin(), any_args.end(), false,
      "{.set_max_nesting 10\n"
      "    .function_all_flags .file_open_failure name stack ... {"
      "        .combine (init file ) $name ( does not exist\n"
      "call stack ) $stack (\n)}\n"
      "    .function_all_flags .raw_command -- args ... {.nop $args}\n"
      "    .source /etc/rwshrc $args$\n"
      "    .for &{.internal_functions}$ {" //.echo $1; .whence_function $1}\n"
      "      .if .test_executable_exists $1 {.nop}\n"
      "      .else {.echo &&1 not defined (\n)}}\n"
      "    .if .test_executable_exists .help {"
      "      .if .test_not_empty ${.help} {.nop}\n"
      "      .else {.echo .help produces no output (\n)}}\n"
      "    .else {.echo .help not defined (\n)}}", exceptions));
  executable_map.set(new Function(".internal_features",
      empty_prototype.begin(), empty_prototype.end(), false,
      "{.echo (.after_command .before_command .run_logic\n)}", exceptions));
  executable_map.set(new Builtin(".internal_functions", b_internal_functions));
  executable_map.set(new Function(".internal_vars",
      empty_prototype.begin(), empty_prototype.end(), false,
      "{.echo (FIGNORE ?\n)}", exceptions));
  executable_map.set(new Builtin(".is_default_input", b_is_default_input));
  executable_map.set(new Builtin(".is_default_output", b_is_default_output));
  executable_map.set(new Builtin(".is_default_error", b_is_default_error));
  executable_map.set(new Builtin(".last_exception",
                                 b_last_exception));
  executable_map.set(new Builtin(".last_execution_time",
                                 b_last_execution_time));
  executable_map.set(new Builtin(".list_environment", b_list_environment));
  executable_map.set(new Builtin(".list_executables", b_list_executables));
  executable_map.set(new Builtin(".list_locals", b_list_locals));
  executable_map.set(new Builtin(".local", b_local));
  executable_map.set(new Builtin(".ls", b_ls));
  executable_map.set(new Builtin(".nop", b_nop));
  executable_map.set(new Builtin(".replace_exception", b_replace_exception));
  executable_map.set(new Builtin(".return", b_return));
  executable_map.set(new Builtin(".rm_executable", b_rm_executable));
  executable_map.set(new Builtin(".scope", b_scope));
  executable_map.set(new Builtin(".selection_set", b_selection_set));
  executable_map.set(new Builtin(".set", b_set));
  executable_map.set(new Builtin(".set_fallback_message",
                                 b_set_fallback_message));
  executable_map.set(new Builtin(".set_max_collectible_exceptions",
                                 b_set_max_collectible_exceptions));
  executable_map.set(new Builtin(".set_max_extra_exceptions",
                                 b_set_max_extra_exceptions));
  executable_map.set(new Builtin(".set_max_nesting", b_set_max_nesting));
  executable_map.set(new Builtin(".source", b_source));
  executable_map.set(new Builtin(".stepwise", b_stepwise));
  executable_map.set(new Builtin(".store_output", b_store_output));
  executable_map.set(new Builtin(".test_executable_exists",
                                 b_test_executable_exists));
  executable_map.set(new Builtin(".test_file_exists", b_test_file_exists));
  executable_map.set(new Builtin(".test_greater", b_test_greater));
  executable_map.set(new Builtin(".test_is_number", b_test_is_number));
  executable_map.set(new Builtin(".test_in", b_test_in));
  executable_map.set(new Builtin(".test_less", b_test_less));
  executable_map.set(new Builtin(".test_not_empty", b_test_not_empty));
  executable_map.set(new Builtin(".test_number_equal", b_test_number_equal));
  executable_map.set(new Builtin(".test_string_equal", b_test_string_equal));
  executable_map.set(new Builtin(".test_string_unequal",
                                 b_test_string_unequal));
  executable_map.set(new Builtin(".throw", b_throw));
  executable_map.set(new Builtin(".toggle_readline", b_toggle_readline));
  executable_map.set(new Builtin(".total_execution_time",
                                 b_total_execution_time));
  executable_map.set(new Builtin(".try_catch_recursive",
                                 b_try_catch_recursive));
  executable_map.set(new Builtin(".type", b_type));
  executable_map.set(new Builtin(".unset", b_unset));
  executable_map.set(new Builtin(".usleep", b_usleep));
  executable_map.set(new Builtin(".usleep_overhead", b_usleep_overhead));
  executable_map.set(new Builtin(".waiting_for_binary", b_waiting_for_binary));
  executable_map.set(new Builtin(".waiting_for_shell", b_waiting_for_shell));
  executable_map.set(new Builtin(".waiting_for_user", b_waiting_for_user));
  executable_map.set(new Builtin(".whence_function", b_whence_function));
  executable_map.set(new Builtin(".which_path", b_which_path));
  executable_map.set(new Builtin(".while", b_while));
  executable_map.set(new Builtin(".var_add", b_var_add));
  executable_map.set(new Builtin(".var_divide", b_var_divide));
  executable_map.set(new Builtin(".var_exists", b_var_exists));
  executable_map.set(new Builtin(".var_subtract", b_var_subtract));
  executable_map.set(new Builtin(".version", b_version));
  executable_map.set(new Builtin(".version_compatible", b_version_compatible));}

