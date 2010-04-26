// the registering of builtins
// Copyright Samuel Newbold 2005-2010

#include <map>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "arg_script.h"
#include "argv.h"
#include "rwsh_init.h"
#include "builtin.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"

void internal_init(void) {
  executable_map.set(new Builtin(".cd", b_cd));
  executable_map.set(new Builtin(".combine", b_combine));
  executable_map.set(new Builtin(".echo", b_echo));
  executable_map.set(new Builtin(".else", b_else));
  executable_map.set(new Builtin(".else_if", b_else_if));
  executable_map.set(new Builtin(".else_if_not", b_else_if_not));
  executable_map.set(new Builtin(".exec", b_exec));
  executable_map.set(new Builtin(".exit", b_exit));
  executable_map.set(new Builtin(".for", b_for));
  executable_map.set(new Builtin(".for_each_line", b_for_each_line));
  executable_map.set(new Builtin(".fork", b_fork));
  executable_map.set(new Builtin(".function", b_function));
  executable_map.set(new Builtin(".global", b_global));
  executable_map.set(new Builtin(".if", b_if));
  executable_map.set(new Builtin(".importenv_overwrite", b_importenv_overwrite));
  executable_map.set(new Builtin(".importenv_preserve", b_importenv_preserve));
  std::string::size_type point = 0;
  executable_map.set(new Function(".init", 
      "{.set MAX_NESTING 4\n"
          ".source /etc/rwshrc $*}", point, 0));
  point = 0;
  executable_map.set(new Function(".internal_errors", 
      "{.if .test_number_equal $# 1 {"
          ".echo rwsh.arguments_for_argfunction rwsh.bad_argfunction_style "
          "rwsh.binary_not_found rwsh.double_redirection "
          "rwsh.excessive_nesting rwsh.executable_not_found "
          "rwsh.failed_substitution rwsh.invalid_word_selection "
          "rwsh.mismatched_brace rwsh.multiple_argfunctions "
          "rwsh.not_soon_enough rwsh.selection_not_found rwsh.sighup "
          "rwsh.sigint rwsh.sigquit rwsh.sigpipe rwsh.sigterm rwsh.sigtstp "
          "rwsh.sigusr1 rwsh.sigusr2 rwsh.undefined_variable "
          "rwsh.unreadable_dir}; "
      ".else {.echo wrong argument count; .return -1}}", point, 0));
  point = 0;
  executable_map.set(new Function(".internal_features", 
      "{.if .test_number_equal $# 1 {"
          ".echo rwsh.after_command rwsh.before_command "
          "rwsh.prompt rwsh.raw_command rwsh.run_logic "
          "rwsh.shutdown rwsh.vars}; "
      ".else {.echo wrong argument count; .return -1}}", point, 0));
  point = 0;
  executable_map.set(new Function(".internal_vars", 
      "{.if .test_number_equal $# 1 {"
          ".echo FIGNORE IF_TEST MAX_NESTING}; "
      ".else {.echo wrong argument count; .return -1}}", point, 0));
  executable_map.set(new Builtin(".is_default_input", b_is_default_input));
  executable_map.set(new Builtin(".is_default_output", b_is_default_output));
  executable_map.set(new Builtin(".is_default_error", b_is_default_error));
  executable_map.set(new Builtin(".ls", b_ls));
  executable_map.set(new Builtin(".nop", b_nop));
  executable_map.set(new Builtin(".return", b_return));
  executable_map.set(new Builtin(".set", b_set));
  executable_map.set(new Builtin(".selection_set", b_selection_set));
  executable_map.set(new Builtin(".source", b_source));
  executable_map.set(new Builtin(".stepwise", b_stepwise));
  executable_map.set(new Builtin(".store_output", b_store_output));
  executable_map.set(new Builtin(".test_string_equal", b_test_string_equal));
  executable_map.set(new Builtin(".test_greater", b_test_greater));
  executable_map.set(new Builtin(".test_is_number", b_test_is_number));
  executable_map.set(new Builtin(".test_less", b_test_less));
  executable_map.set(new Builtin(".test_not_empty", b_test_not_empty));
  executable_map.set(new Builtin(".test_string_unequal", b_test_string_unequal));
  executable_map.set(new Builtin(".test_number_equal", b_test_number_equal));
  executable_map.set(new Builtin(".unset", b_unset));
  executable_map.set(new Builtin(".usleep", b_usleep));
  executable_map.set(new Builtin(".b_waiting_for",
                                   b_waiting_for));
  executable_map.set(new Builtin(".waiting_for_shell", b_waiting_for_shell));
  executable_map.set(new Builtin(".waiting_for_user", b_waiting_for_user));
  executable_map.set(new Builtin(".which_executable", b_which_executable));
  executable_map.set(new Builtin(".which_execution_count",
                                   b_which_execution_count));
  executable_map.set(new Builtin(".which_last_execution_time",
                                   b_which_last_execution_time));
  executable_map.set(new Builtin(".which_total_execution_time",
                                   b_which_total_execution_time));
  executable_map.set(new Builtin(".which_path", b_which_path));
  executable_map.set(new Builtin(".which_return", b_which_return));
  executable_map.set(new Builtin(".which_test", b_which_test));
  executable_map.set(new Builtin(".while", b_while));
  executable_map.set(new Builtin(".var_add", b_var_add));
  executable_map.set(new Builtin(".var_subtract", b_var_subtract));
  executable_map.set(new Builtin(".var_divide", b_var_divide));
  executable_map.set(new Builtin(".var_exists", b_var_exists));
  executable_map.set(new Builtin(".version", b_version));
  executable_map.set(new Builtin(".version_available", b_version_available));
  executable_map.set(new Builtin(".version_compatible", b_version_compatible));}

