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
  ".ambiguous_prototype_dash_dash",
  ".arguments_for_argfunction",
  ".autofunction",
  ".bad_argc",
  ".bad_argfunction_style",
  ".bad_args",
  ".bad_if_nest",
  ".binary_does_not_exist",
  ".binary_not_found",
  ".dash_dash_argument",
  ".dash_star_argument",
  ".directory_not_found",
  ".divide_by_zero",
  ".double_redirection",
  ".duplicate_parameter",
  ".elipsis_first_arg",
  ".elipsis_out_of_option_group",
  ".else_without_if",
  ".excess_argfunction",
  ".excessive_exceptions_collected",
  ".excessive_exceptions_in_catch",
  ".excessive_nesting",
  ".exec_failed",
  ".executable_already_exists",
  ".failed_substitution",
  ".false",
  ".file_open_failure",
  ".file_not_found",
  ".flag_in_elipsis",
  ".function_not_found",
  ".global_would_be_masked",
  ".if_before_else",
  ".illegal_function_name",
  ".illegal_variable_name",
  ".input_range",
  ".internal_error",
  ".invalid_word_selection",
  // ".line_continuation",
  ".mismatched_brace",
  ".mismatched_bracket",
  ".mismatched_parenthesis",
  ".missing_argfunction",
  ".multiple_argfunctions",
  ".not_a_directory",
  ".not_a_function",
  ".not_a_number",
  ".not_executable",
  ".not_soon_enough",
  ".raw_command",
  ".return_code",
  ".result_range",
  ".post_elipsis_option",
  ".post_dash_dash_flag",
  ".prompt",
  ".selection_not_found",
  ".shutdown",
  ".sighup",
  ".sigint",
  ".sigquit",
  ".sigpipe",
  ".sigterm",
  ".sigtstp",
  ".sigcont",
  ".sigchld",
  ".sigusr1",
  ".sigusr2",
  ".sigunknown",
  ".tardy_flag",
  ".unchecked_variable",
  ".undeclared_variable",
  ".undefined_variable",
  ".unreadable_dir",
  ".unrecognized_flag",
  ".unused_before_set",
  ".unused_variable",
  ".variable_already_exists",
  ".version_incompatible"};
bool readline_enabled = false;
Variable_map root_variable_map(nullptr);
int Base_executable::max_nesting = 0;
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
  Error_list exceptions;
  try {internal_init(exceptions);}                   // catch blocks untestable
  catch (std::string& error) {default_error <<error;}
  catch (Exception& exception) {
    executable_map.base_run(exception, exceptions);}
  Command_stream command_stream(default_input, true);
  Argm::Argv std_argv(&argv[0], &argv[argc]);
  Argm init_command(".init", std_argv,
                    nullptr, Variable_map::global_map,
                    default_input, default_output, default_error);
  executable_map.base_run(init_command, exceptions);
  register_signals();
  Arg_script script("", 0, exceptions);
  Exception prompt(Argm::Prompt);
  while (!command_stream.fail()) {
    executable_map.base_run(prompt, exceptions);
    Argm command(Variable_map::global_map,
                 default_input, default_output, default_error);
    try {
      command_stream.getline(script, exceptions);
      if (exceptions.size()) {
        Base_executable::exception_handler(exceptions);
        continue;}
      else if (command_stream.fail()) break;
      else command = script.base_interpret(script.argm(), exceptions);}
    catch (Exception exception) {command = exception;}
    executable_map.run_if_exists(".before_command", command);
    if (!executable_map.run_if_exists(".run_logic", command))
       executable_map.base_run(command, exceptions);
    executable_map.run_if_exists(".after_command", command);}
  Argm shutdown_command(Argm::exception_names[Argm::Shutdown],
                        std_argv, nullptr, Variable_map::global_map,
                        default_input, default_output, default_error);
  executable_map.base_run(shutdown_command, exceptions);
  executable_map.unused_var_check_at_exit();
  return Variable_map::dollar_question;}
