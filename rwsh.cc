// The main function for rwsh
//
// Copyright (C) 2005-2018 Samuel Newbold

#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <signal.h>
#include <string>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "clock.h"
#include "command_stream.h"
#include "default_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "plumber.h"
#include "prototype.h"
#include "rwsh_init.h"
#include "selection.h"

#include "function.h"
#include "argm_star_var.cc"

// static initializers of basic types
const char* WSPACE = " \t";
bool Base_executable::collect_excess_thrown = false;
unsigned Base_executable::max_collect = 1;
unsigned Base_executable::max_extra = 1;
bool Base_executable::execution_handler_excess_thrown = false;
struct timezone Clock::no_timezone_v = {0, 0};
int Base_executable::global_nesting(0);
bool Base_executable::unwind_stack_v(false);
bool Base_executable::in_exception_handler(false);
unsigned Base_executable::current_exception_count(0);
unsigned Base_executable::dropped_catches(0);
std::string Argm::exception_names[Argm::Exception_count] = {
  "no exception",
  "rwsh.ambiguous_prototype_dash_dash",
  "rwsh.arguments_for_argfunction",
  "rwsh.bad_argc",
  "rwsh.bad_argfunction_style",
  "rwsh.bad_args",
  "rwsh.bad_if_nest",
  "rwsh.binary_not_found",
  "rwsh.dash_dash_argument",
  "rwsh.dash_star_argument",
  "rwsh.divide_by_zero",
  "rwsh.double_redirection",
  "rwsh.duplicate_parameter",
  "rwsh.elipsis_first_arg",
  "rwsh.elipsis_out_of_option_group",
  "rwsh.else_without_if",
  "rwsh.excess_argfunction",
  "rwsh.excessive_exceptions_collected",
  "rwsh.excessive_exceptions_in_catch",
  "rwsh.excessive_nesting",
  "rwsh.executable_not_found",
  "rwsh.failed_substitution",
  "rwsh.file_open_failure",
  "rwsh.flag_in_elipsis",
  "rwsh.if_before_else",
  "rwsh.input_range",
  "rwsh.internal_error",
  "rwsh.invalid_word_selection",
  // "rwsh.line_continuation",
  "rwsh.mismatched_brace",
  "rwsh.mismatched_bracket",
  "rwsh.mismatched_parenthesis",
  "rwsh.missing_argfunction",
  "rwsh.multiple_argfunctions",
  "rwsh.not_a_number",
  "rwsh.not_executable",
  "rwsh.not_soon_enough",
  "rwsh.raw_command",
  "rwsh.return_code",
  "rwsh.result_range",
  "rwsh.post_elipsis_option",
  "rwsh.post_dash_dash_flag",
  "rwsh.prompt",
  "rwsh.selection_not_found",
  "rwsh.shutdown",
  "rwsh.sighup",
  "rwsh.sigint",
  "rwsh.sigquit",
  "rwsh.sigpipe",
  "rwsh.sigterm",
  "rwsh.sigtstp",
  "rwsh.sigcont",
  "rwsh.sigchld",
  "rwsh.sigusr1",
  "rwsh.sigusr2",
  "rwsh.sigunknown",
  "rwsh.tardy_flag",
  "rwsh.unchecked_variable",
  "rwsh.undefined_variable",
  "rwsh.unreadable_dir",
  "rwsh.unrecognized_flag",
  "rwsh.unused_variable",
  "rwsh.vars",
  "rwsh.version_incompatible"};
bool readline_enabled = false;
Variable_map root_variable_map(NULL);
unsigned Base_executable::max_nesting = 0;
int Variable_map::dollar_question = -1;
bool Variable_map::exit_requested = false;

// static initializers without dependancies
Clock rwsh_clock;
Executable_map executable_map;
Plumber plumber;
Rwsh_istream_p default_input(new Default_istream(0), true, true);
Rwsh_ostream_p default_output(new Default_ostream(1), true, true),
  default_error(new Default_ostream(2), true, true);
Variable_map* Variable_map::global_map = &root_variable_map;

// static initializers with cross-component dependancies
Argm::Exception_t Base_executable::caught_signal = Argm::No_exception;

namespace {
void register_signals(void) {
  signal(SIGHUP, Named_executable::unix_signal_handler);
  signal(SIGINT, Named_executable::unix_signal_handler);
  signal(SIGQUIT, Named_executable::unix_signal_handler);
  signal(SIGPIPE, Named_executable::unix_signal_handler);
  signal(SIGTERM, Named_executable::unix_signal_handler);
  signal(SIGTSTP, Named_executable::unix_signal_handler);
  signal(SIGUSR1, Named_executable::unix_signal_handler);
  signal(SIGUSR2, Named_executable::unix_signal_handler);}
} // end unnamed namespace

int main(int argc, char *argv[]) {
  try {internal_init();}                             // catch blocks untestable
  catch (std::string& error) {default_error <<error;}
  catch (Exception& exception) {
    executable_map.base_run(exception);}
  Command_stream command_stream(default_input, true);
  Argm init_command(".init", &argv[0], &argv[argc], 0, Variable_map::global_map,
                             default_input, default_output, default_error);
  executable_map.base_run(init_command);
  register_signals();
  Arg_script script("", 0);
  Exception prompt(Argm::Prompt);
  while (!command_stream.fail()) {
    executable_map.base_run(prompt);
    Argm command(Variable_map::global_map,
                 default_input, default_output, default_error);
    try {
      command_stream >> script;
      if (command_stream.fail()) break;
      command = script.base_interpret(script.argm());}
    catch (Exception exception) {command = exception;}
    executable_map.run_if_exists("rwsh.before_command", command);
    if (!executable_map.run_if_exists("rwsh.run_logic", command))
       executable_map.base_run(command);
    executable_map.run_if_exists("rwsh.after_command", command);}
  Argm shutdown_command(Argm::exception_names[Argm::Shutdown],
                        &argv[0], &argv[argc], 0, Variable_map::global_map,
                        default_input, default_output, default_error);
  executable_map.base_run(shutdown_command);
  executable_map.unused_var_check_at_exit();
  return Variable_map::dollar_question;}
