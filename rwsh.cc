// The main function for rwsh
//
// Copyright (C) 2005-2008 Samuel Newbold

#include <iostream>
#include <list>
#include <map>
#include <signal.h>
#include <string>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argv.h"
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
#include "variable_map.h"

#include "argv_star_var.cc"

// static initializers of basic types
struct timezone Clock::no_timezone_v = {0, 0};
int Executable::global_nesting(0);
bool Executable::caught_signal(false);
bool Executable::in_signal_handler(false);
std::string Argv::signal_names[Argv::Signal_count] = {
  "rwsh.argument_count",
  "rwsh.arguments_for_argfunction",
  "rwsh.bad_argfunction_style",
  "rwsh.bad_if_nest", 
  "rwsh.binary_not_found",
  "rwsh.divide_by_zero",
  "rwsh.double_redirection",
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
  "rwsh.mismatched_parenthesis",
  "rwsh.missing_argfunction",
  "rwsh.multiple_argfunctions",
  "rwsh.not_a_number",
  "rwsh.not_executable",
  "rwsh.not_soon_enough",
  "rwsh.result_range",
  "rwsh.prompt",
  "rwsh.selection_not_found",
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
  "rwsh.version_incompatible"};

// static initializers without dependancies
Clock rwsh_clock;
Executable_map executable_map;
Plumber plumber;
Rwsh_istream_p default_input(new Default_istream(0), true, true);
Rwsh_ostream_p default_output(new Default_ostream(1), true, true),
  default_error(new Default_ostream(2), true, true);
Variable_map root_variable_map(true);
Variable_map* vars = &root_variable_map;
int Variable_map::dollar_question = -1;
int& dollar_question = Variable_map::dollar_question;
bool Variable_map::exit_requested = false;

// static initializers with cross-component dependancies
Argv Executable::call_stack;
Variable_map* Argv::var_map = vars;

namespace {
void register_signals(void) {
  signal(SIGHUP, Executable::unix_signal_handler);
  signal(SIGINT, Executable::unix_signal_handler);
  signal(SIGQUIT, Executable::unix_signal_handler);
  signal(SIGPIPE, Executable::unix_signal_handler);
  signal(SIGTERM, Executable::unix_signal_handler);
  signal(SIGTSTP, Executable::unix_signal_handler);
  signal(SIGUSR1, Executable::unix_signal_handler);
  signal(SIGUSR2, Executable::unix_signal_handler);} } // end unnamed namespace

int main(int argc, char *argv[]) {
  Argv external_command_line(&argv[0], &argv[argc], 0, 
                               default_input, default_output, default_error);
  try {internal_init();}
  catch (std::string& error) {default_error <<error;}
  catch (Signal_argv& exception) {executable_map.run(exception);}
  Command_stream command_stream(std::cin, true);
  executable_map.run_if_exists(".init", external_command_line);
  register_signals();
  Arg_script script("", 0);
  Argv prompt;
  prompt.push_back("rwsh.prompt");
  while (command_stream) {
    executable_map.run(prompt);
    Argv command;
    try {
      if (!(command_stream >> script)) break;
      command = script.interpret(script.argv());}
    catch (Signal_argv exception) {command = exception;}
    executable_map.run_if_exists("rwsh.before_command", command);
    if (!executable_map.run_if_exists("rwsh.run_logic", command))
       executable_map.run(command);
    executable_map.run_if_exists("rwsh.after_command", command);}
  executable_map.run_if_exists("rwsh.shutdown", external_command_line);
  return dollar_question;}
