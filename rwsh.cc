// The main function for rwsh and the definition of rwsh.init
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <iostream>
#include <map>
#include <signal.h>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "builtin.h"
#include "clock.h"
#include "command_stream.h"
#include "default_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "plumber.h"
#include "selection.h"
#include "variable_map.h"

struct timezone Clock::no_timezone_v = {0, 0};
Clock rwsh_clock;
Rwsh_istream_p default_input(new Default_istream_t(0), true, true);
Rwsh_ostream_p default_output(new Default_ostream_t(1), true, true),
  default_error(new Default_ostream_t(2), true, true);
Executable_map_t executable_map;
Plumber plumber;

namespace {
std::string init_str =
  "%set MAX_NESTING 4;"
  "%source /etc/rwshrc $*";

void internal_init(void) {
  executable_map.set(new Builtin_t("%append_to_errno", append_to_errno_bi));
  executable_map.set(new Builtin_t("%cd", cd_bi));
  executable_map.set(new Builtin_t("%echo", echo_bi));
  executable_map.set(new Builtin_t("%else", else_bi));
  executable_map.set(new Builtin_t("%else_if", else_if_bi));
  executable_map.set(new Builtin_t("%else_if_not", else_if_not_bi));
  executable_map.set(new Builtin_t("%error_unit", error_unit_bi));
  executable_map.set(new Builtin_t("%exit", exit_bi));
  executable_map.set(new Builtin_t("%for", for_bi));
  executable_map.set(new Builtin_t("%for_each_line", for_each_line_bi));
  executable_map.set(new Builtin_t("%function", function_bi));
  executable_map.set(new Builtin_t("%global", global_bi));
  executable_map.set(new Builtin_t("%if", if_bi));
  executable_map.set(new Builtin_t("%if_errno", if_errno_bi));
  executable_map.set(new Builtin_t("%if_errno_is", if_errno_is_bi));
  executable_map.set(new Builtin_t("%importenv_overwrite", importenv_overwrite_bi));
  executable_map.set(new Builtin_t("%importenv_preserve", importenv_preserve_bi));
  executable_map.set(new Function_t("%internal_errors", 
      "%error_unit $* {"
          "%if %test_equal $# 1 {"
              "%echo rwsh.arguments_for_argfunction rwsh.bad_argfunction_style "
              "rwsh.binary_not_found rwsh.double_redirection "
              "rwsh.excessive_nesting rwsh.executable_not_found "
              "rwsh.failed_substitution rwsh.mismatched_brace "
              "rwsh.multiple_argfunctions rwsh.not_soon_enough rwsh.init "
              "rwsh.selection_not_found rwsh.sighup rwsh.sigint "
              "rwsh.sigquit rwsh.sigpipe rwsh.sigterm "
              "rwsh.sigtstp rwsh.siginfo rwsh.sigusr1 rwsh.sigusr2 "
              "rwsh.undefined_variable rwsh.unreadable_dir}; "
          "%else {%append_to_errno ARGS; %return -1}}", 0));
  executable_map.set(new Function_t("%internal_features", 
      "%error_unit $* {"
          "%if %test_equal $# 1 {"
              "%echo rwsh.after_command rwsh.before_command "
              "rwsh.prompt rwsh.raw_command rwsh.run_logic "
              "rwsh.shutdown rwsh.vars}; "
          "%else {%append_to_errno ARGS; %return -1}}", 0));
  executable_map.set(new Function_t("%internal_vars", 
      "%error_unit $* {"
          "%if %test_equal $# 1 {"
              "%echo CWD ERRNO FIGNORE IF_TEST MAX_NESTING}; "
          "%else {%append_to_errno ARGS; %return -1}}", 0));
  executable_map.set(new Builtin_t("%is_default_input", is_default_input_bi));
  executable_map.set(new Builtin_t("%is_default_output", is_default_output_bi));
  executable_map.set(new Builtin_t("%is_default_error", is_default_error_bi));
  executable_map.set(new Builtin_t("%ls", ls_bi));
  executable_map.set(new Builtin_t("%newline", newline_bi));
  executable_map.set(new Builtin_t("%nop", nop_bi));
  executable_map.set(new Builtin_t("%return", return_bi));
  executable_map.set(new Builtin_t("%set", set_bi));
  executable_map.set(new Builtin_t("%selection_set", selection_set_bi));
  executable_map.set(new Builtin_t("%source", source_bi));
  executable_map.set(new Builtin_t("%stepwise", stepwise_bi));
  executable_map.set(new Builtin_t("%test_equal", test_equal_bi));
  executable_map.set(new Builtin_t("%test_not_empty", test_not_empty_bi));
  executable_map.set(new Builtin_t("%test_not_equal", test_not_equal_bi));
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
  executable_map.set(new Builtin_t("%var_divide", var_divide_bi));
  executable_map.set(new Builtin_t("%var_exists", var_exists_bi));
  executable_map.set(new Builtin_t("%version", version_bi));
  executable_map.set(new Builtin_t("%version_available", version_available_bi));
  executable_map.set(new Builtin_t("%version_compatible", version_compatible_bi));}

void signal_starter(int sig) {Executable_t::caught_signal = sig;}

void register_signals(void) {
  signal(SIGHUP, signal_starter);
  signal(SIGINT, signal_starter);
  signal(SIGQUIT, signal_starter);
  signal(SIGPIPE, signal_starter);
  signal(SIGTERM, signal_starter);
  signal(SIGTSTP, signal_starter);
  signal(SIGINFO, signal_starter);
  signal(SIGUSR1, signal_starter);
  signal(SIGUSR2, signal_starter);} } // end unnamed namespace

int main(int argc, char *argv[]) {
  Argv_t external_command_line(&argv[0], &argv[argc], 0, 
                               default_input, default_output, default_error);
  internal_init();
  Command_stream_t command_stream(std::cin);
  executable_map.set(new Function_t("rwsh.init", init_str, 0));
  executable_map.run_if_exists("rwsh.init", external_command_line);
  register_signals();
  Arg_script_t script("", 0);
  Argv_t prompt;
  while (command_stream) {
    executable_map.run_if_exists("rwsh.prompt", prompt);
    Argv_t command;
    try {
      if (!(command_stream >> script)) break;
      command = script.interpret(script.argv());}
    catch (Argv_t exception) {command = exception;}
    executable_map.run_if_exists("rwsh.before_command", command);
    if (!executable_map.run_if_exists("rwsh.run_logic", command))
       executable_map.run(command);
    executable_map.run_if_exists("rwsh.after_command", command);}
  executable_map.run_if_exists("rwsh.shutdown", external_command_line);
  return dollar_question;}
