// The main function for rwsh
//
// Copyright (C) 2005-2015 Samuel Newbold

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
#include "function.h"
#include "plumber.h"
#include "rwsh_init.h"
#include "selection.h"

#include "argm_star_var.cc"

// static initializers of basic types
struct timezone Clock::no_timezone_v = {0, 0};
int Base_executable::global_nesting(0);
Argm::Sig_type Base_executable::caught_signal(Argm::No_signal);
bool Base_executable::in_signal_handler(false);
std::string Argm::signal_names[Argm::Signal_count] = {
  "no signal",
  "rwsh.arguments_for_argfunction",
  "rwsh.bad_argc",
  "rwsh.bad_argfunction_style",
  "rwsh.bad_if_nest", 
  "rwsh.binary_not_found",
  "rwsh.dash_dash_argument",
  "rwsh.divide_by_zero",
  "rwsh.double_redirection",
  "rwsh.duplicate_parameter",
  "rwsh.else_without_if",
  "rwsh.excess_argfunction",
  "rwsh.excessive_nesting",
  "rwsh.executable_not_found",
  "rwsh.failed_substitution",
  "rwsh.file_open_failure",
  "rwsh.if_before_else",
  "rwsh.input_range",
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
  "rwsh.result_range",
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
  "rwsh.undefined_variable",
  "rwsh.unreadable_dir",
  "rwsh.unrecognized_flag",
  "rwsh.vars",
  "rwsh.version_incompatible"};

// static initializers without dependancies
const char* Arg_script::TOKEN_SEPARATORS = " \t";
Clock rwsh_clock;
Executable_map executable_map;
Plumber plumber;
Rwsh_istream_p default_input(new Default_istream(0), true, true);
Rwsh_ostream_p default_output(new Default_ostream(1), true, true),
  default_error(new Default_ostream(2), true, true);
Variable_map root_variable_map(NULL);
Variable_map* Variable_map::global_map = &root_variable_map;
unsigned Variable_map::max_nesting_v = 0;
int Variable_map::dollar_question = -1;
bool Variable_map::exit_requested = false;

// static initializers with cross-component dependancies
Argm Base_executable::call_stack(Variable_map::global_map,
                            default_input, default_output, default_error);

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
  try {internal_init();}
  catch (std::string& error) {default_error <<error;}
  catch (Signal_argm& exception) {executable_map.run(exception);}
  Command_stream command_stream(std::cin, true);
  Argm init_command(".init", &argv[0], &argv[argc], 0, Variable_map::global_map,
                             default_input, default_output, default_error);
  executable_map.run(init_command);
  register_signals();
  Arg_script script("", 0);
  Signal_argm prompt(Argm::Prompt);
  while (command_stream) {
    executable_map.run(prompt);
    Argm command(Variable_map::global_map,
                 default_input, default_output, default_error);
    try {
      if (!(command_stream >> script)) break;
      command = script.interpret(script.argm());}
    catch (Signal_argm exception) {command = exception;}
    executable_map.run_if_exists("rwsh.before_command", command);
    if (!executable_map.run_if_exists("rwsh.run_logic", command))
       executable_map.run(command);
    executable_map.run_if_exists("rwsh.after_command", command);}
  Argm shutdown_command(Argm::signal_names[Argm::Shutdown],
                        &argv[0], &argv[argc], 0, Variable_map::global_map,
                        default_input, default_output, default_error);
  executable_map.run(shutdown_command);
  return Variable_map::dollar_question;}
