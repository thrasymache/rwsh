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

void internal_init(void) {
  executable_map.set(new Builtin(".argc", b_argc));
  executable_map.set(new Builtin(".cd", b_cd));
  executable_map.set(new Builtin(".collect_errors_except",
                                 b_collect_errors_except));
  executable_map.set(new Builtin(".collect_errors_only",
                                 b_collect_errors_only));
  executable_map.set(new Builtin(".combine", b_combine));
  executable_map.set(new Builtin(".enable_readline", b_enable_readline));
  executable_map.set(new Builtin(".echo", b_echo));
  executable_map.set(new Builtin(".disable_readline", b_disable_readline));
  executable_map.set(new Builtin(".else", b_else));
  executable_map.set(new Builtin(".else_if", b_else_if));
  executable_map.set(new Builtin(".else_if_not", b_else_if_not));
  executable_map.set(new Builtin(".exec", b_exec));
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
      "{.set_max_nesting 10\n"
          ".function rwsh.file_open_failure {.echo init file $1 does not exist "
          "\\(call stack $*2 \\) (\n)}\n"
          ".function rwsh.raw_command {.nop $*}\n"
          ".source /etc/rwshrc $*\n"
          ".for &{.internal_functions}$ {" //.echo $1; .which_executable $1}\n"
            ".if .which_test $1 {.nop}\n"
            ".else {.echo &&1 not defined (\n)}}\n"
          ".if .which_test rwsh.help {"
            ".if .test_not_empty &&{rwsh.help} {.nop}\n"
            ".else {.echo rwsh.help produces no output (\n)}}\n"
          ".else {.echo rwsh.help not defined (\n)}}"));
  executable_map.set(new Function(".internal_features",
      "{.if .test_number_equal $# 1 {"
          ".echo rwsh.after_command rwsh.before_command rwsh.run_logic}; "
      ".else {.echo wrong argument count; .return -1}}"));
  executable_map.set(new Builtin(".internal_functions", b_internal_functions));
  executable_map.set(new Function(".internal_vars",
      "{.if .test_number_equal $# 1 {"
          ".echo FIGNORE ?}; "
      ".else {.echo wrong argument count; .return -1}}"));
  executable_map.set(new Builtin(".is_default_input", b_is_default_input));
  executable_map.set(new Builtin(".is_default_output", b_is_default_output));
  executable_map.set(new Builtin(".is_default_error", b_is_default_error));
  executable_map.set(new Builtin(".list_environment", b_list_environment));
  executable_map.set(new Builtin(".list_executables", b_list_executables));
  executable_map.set(new Builtin(".list_locals", b_list_locals));
  executable_map.set(new Builtin(".local", b_local));
  executable_map.set(new Builtin(".ls", b_ls));
  executable_map.set(new Builtin(".nop", b_nop));
  executable_map.set(new Builtin(".return", b_return));
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
  executable_map.set(new Builtin(".test_greater", b_test_greater));
  executable_map.set(new Builtin(".test_is_number", b_test_is_number));
  executable_map.set(new Builtin(".test_less", b_test_less));
  executable_map.set(new Builtin(".test_not_empty", b_test_not_empty));
  executable_map.set(new Builtin(".test_number_equal", b_test_number_equal));
  executable_map.set(new Builtin(".test_string_equal", b_test_string_equal));
  executable_map.set(new Builtin(".test_string_unequal",
                                 b_test_string_unequal));
  executable_map.set(new Builtin(".throw", b_throw));
  executable_map.set(new Builtin(".toggle_readline", b_toggle_readline));
  executable_map.set(new Builtin(".try_catch_recursive",
                                 b_try_catch_recursive));
  executable_map.set(new Builtin(".unset", b_unset));
  executable_map.set(new Builtin(".usleep", b_usleep));
  executable_map.set(new Builtin(".usleep_overhead", b_usleep_overhead));
  executable_map.set(new Builtin(".waiting_for_binary", b_waiting_for_binary));
  executable_map.set(new Builtin(".waiting_for_shell", b_waiting_for_shell));
  executable_map.set(new Builtin(".waiting_for_user", b_waiting_for_user));
  executable_map.set(new Builtin(".which_executable", b_which_executable));
  executable_map.set(new Builtin(".which_execution_count",
                                 b_which_execution_count));
  executable_map.set(new Builtin(".which_last_exception",
                                 b_which_last_exception));
  executable_map.set(new Builtin(".which_last_execution_time",
                                 b_which_last_execution_time));
  executable_map.set(new Builtin(".which_path", b_which_path));
  executable_map.set(new Builtin(".which_return", b_which_return));
  executable_map.set(new Builtin(".which_test", b_which_test));
  executable_map.set(new Builtin(".which_total_execution_time",
                                 b_which_total_execution_time));
  executable_map.set(new Builtin(".while", b_while));
  executable_map.set(new Builtin(".var_add", b_var_add));
  executable_map.set(new Builtin(".var_divide", b_var_divide));
  executable_map.set(new Builtin(".var_exists", b_var_exists));
  executable_map.set(new Builtin(".var_subtract", b_var_subtract));
  executable_map.set(new Builtin(".version", b_version));
  executable_map.set(new Builtin(".version_compatible", b_version_compatible));}

