// the definition of rwsh.init
// Copyright Samuel Newbold 2005-2008

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

std::string init_str =
  "{%set MAX_NESTING 4\n"
   "%source /etc/rwshrc $*}";

void internal_init(void) {
  executable_map.set(new Builtin_t("%append_to_errno", append_to_errno_bi));
  executable_map.set(new Builtin_t("%cd", cd_bi));
  executable_map.set(new Builtin_t("%combine", combine_bi));
  executable_map.set(new Builtin_t("%echo", echo_bi));
  executable_map.set(new Builtin_t("%else", else_bi));
  executable_map.set(new Builtin_t("%else_if", else_if_bi));
  executable_map.set(new Builtin_t("%else_if_not", else_if_not_bi));
  executable_map.set(new Builtin_t("%error_unit", error_unit_bi));
  executable_map.set(new Builtin_t("%exec", exec_bi));
  executable_map.set(new Builtin_t("%exit", exit_bi));
  executable_map.set(new Builtin_t("%for", for_bi));
  executable_map.set(new Builtin_t("%for_each_line", for_each_line_bi));
  executable_map.set(new Builtin_t("%fork", fork_bi));
  executable_map.set(new Builtin_t("%function", function_bi));
  executable_map.set(new Builtin_t("%global", global_bi));
  executable_map.set(new Builtin_t("%if", if_bi));
  executable_map.set(new Builtin_t("%if_errno", if_errno_bi));
  executable_map.set(new Builtin_t("%if_errno_is", if_errno_is_bi));
  executable_map.set(new Builtin_t("%importenv_overwrite", importenv_overwrite_bi));
  executable_map.set(new Builtin_t("%importenv_preserve", importenv_preserve_bi));
  std::string::size_type point = 0;
  executable_map.set(new Function_t("%internal_errors", 
      "{%error_unit $* {"
          "%if %test_equal $# 1 {"
              "%echo rwsh.arguments_for_argfunction rwsh.bad_argfunction_style "
              "rwsh.binary_not_found rwsh.double_redirection "
              "rwsh.excessive_nesting rwsh.executable_not_found "
              "rwsh.failed_substitution rwsh.invalid_word_selection "
              "rwsh.mismatched_brace rwsh.multiple_argfunctions "
              "rwsh.not_soon_enough rwsh.init rwsh.selection_not_found "
              "rwsh.sighup rwsh.sigint rwsh.sigquit rwsh.sigpipe rwsh.sigterm "
              "rwsh.sigtstp rwsh.sigusr1 rwsh.sigusr2 "
              "rwsh.undefined_variable rwsh.unreadable_dir}; "
          "%else {%append_to_errno ARGS; %return -1}}}", point, 0));
  point = 0;
  executable_map.set(new Function_t("%internal_features", 
      "{%error_unit $* {"
          "%if %test_equal $# 1 {"
              "%echo rwsh.after_command rwsh.before_command "
              "rwsh.prompt rwsh.raw_command rwsh.run_logic "
              "rwsh.shutdown rwsh.vars}; "
          "%else {%append_to_errno ARGS; %return -1}}}", point, 0));
  point = 0;
  executable_map.set(new Function_t("%internal_vars", 
      "{%error_unit $* {"
          "%if %test_equal $# 1 {"
              "%echo CWD ERRNO FIGNORE IF_TEST MAX_NESTING}; "
          "%else {%append_to_errno ARGS; %return -1}}}", point, 0));
  executable_map.set(new Builtin_t("%is_default_input", is_default_input_bi));
  executable_map.set(new Builtin_t("%is_default_output", is_default_output_bi));
  executable_map.set(new Builtin_t("%is_default_error", is_default_error_bi));
  executable_map.set(new Builtin_t("%ls", ls_bi));
  executable_map.set(new Builtin_t("%nop", nop_bi));
  executable_map.set(new Builtin_t("%return", return_bi));
  executable_map.set(new Builtin_t("%set", set_bi));
  executable_map.set(new Builtin_t("%selection_set", selection_set_bi));
  executable_map.set(new Builtin_t("%source", source_bi));
  executable_map.set(new Builtin_t("%stepwise", stepwise_bi));
  executable_map.set(new Builtin_t("%test_equal", test_equal_bi));
  executable_map.set(new Builtin_t("%test_greater", test_greater_bi));
  executable_map.set(new Builtin_t("%test_is_number", test_is_number_bi));
  executable_map.set(new Builtin_t("%test_less", test_less_bi));
  executable_map.set(new Builtin_t("%test_not_empty", test_not_empty_bi));
  executable_map.set(new Builtin_t("%test_not_equal", test_not_equal_bi));
  executable_map.set(new Builtin_t("%test_number_equal", test_number_equal_bi));
  executable_map.set(new Builtin_t("%unset", unset_bi));
  executable_map.set(new Builtin_t("%waiting_for_binary",
                                   waiting_for_binary_bi));
  executable_map.set(new Builtin_t("%waiting_for_shell", waiting_for_shell_bi));
  executable_map.set(new Builtin_t("%waiting_for_user", waiting_for_user_bi));
  executable_map.set(new Builtin_t("%which_executable", which_executable_bi));
  executable_map.set(new Builtin_t("%which_execution_count",
                                   which_execution_count_bi));
  executable_map.set(new Builtin_t("%which_last_execution_time",
                                   which_last_execution_time_bi));
  executable_map.set(new Builtin_t("%which_total_execution_time",
                                   which_total_execution_time_bi));
  executable_map.set(new Builtin_t("%which_path", which_path_bi));
  executable_map.set(new Builtin_t("%which_return", which_return_bi));
  executable_map.set(new Builtin_t("%which_test", which_test_bi));
  executable_map.set(new Builtin_t("%while", while_bi));
  executable_map.set(new Builtin_t("%var_add", var_add_bi));
  executable_map.set(new Builtin_t("%var_subtract", var_subtract_bi));
  executable_map.set(new Builtin_t("%var_divide", var_divide_bi));
  executable_map.set(new Builtin_t("%var_exists", var_exists_bi));
  executable_map.set(new Builtin_t("%version", version_bi));
  executable_map.set(new Builtin_t("%version_available", version_available_bi));
  executable_map.set(new Builtin_t("%version_compatible", version_compatible_bi));}

