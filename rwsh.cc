// The main function for rwsh
//
// Copyright (C) 2005-2008 Samuel Newbold

#include <iostream>
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
int Executable_t::global_nesting(0);
int Executable_t::caught_signal(0);
bool Executable_t::in_signal_handler(false);

// static initializers without dependancies
Clock rwsh_clock;
Executable_map_t executable_map;
Plumber plumber;
Rwsh_istream_p default_input(new Default_istream_t(0), true, true);
Rwsh_ostream_p default_output(new Default_ostream_t(1), true, true),
  default_error(new Default_ostream_t(2), true, true);
Variable_map_t root_variable_map(true);
Variable_map_t* vars = &root_variable_map;
int Variable_map_t::dollar_question = -1;
int& dollar_question = Variable_map_t::dollar_question;
bool Variable_map_t::exit_requested = false;

// static initializers with cross-component dependancies
Argv_t Executable_t::call_stack;
Variable_map_t* Argv_t::var_map = vars;

namespace {
void signal_starter(int sig) {Executable_t::caught_signal = sig;}

void register_signals(void) {
  signal(SIGHUP, signal_starter);
  signal(SIGINT, signal_starter);
  signal(SIGQUIT, signal_starter);
  signal(SIGPIPE, signal_starter);
  signal(SIGTERM, signal_starter);
  signal(SIGTSTP, signal_starter);
  signal(SIGUSR1, signal_starter);
  signal(SIGUSR2, signal_starter);} } // end unnamed namespace

int main(int argc, char *argv[]) {
  Argv_t external_command_line(&argv[0], &argv[argc], 0, 
                               default_input, default_output, default_error);
  internal_init();
  Command_stream_t command_stream(std::cin, true);
  std::string::size_type point = 0;
  executable_map.set(new Function_t("rwsh.init", init_str, point, 0));
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
