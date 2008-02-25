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
#include "command_stream.h"
#include "selection.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "variable_map.h"

Executable_map_t executable_map;

namespace {
std::string init_str =
  "%set MAX_NESTING 4;"
  "%if %test_equal $# 3 {%source &2};"
  "%else {%source /etc/rwshrc}";

void signal_starter(int sig) {Executable_t::caught_signal = sig;}

void register_signals(void) {
  signal(SIGHUP, signal_starter);
  signal(SIGINT, signal_starter);
  signal(SIGQUIT, signal_starter);
  signal(SIGPIPE, signal_starter);
  signal(SIGTERM, signal_starter);
  signal(SIGTSTP, signal_starter);
  signal(SIGCONT, signal_starter);
  signal(SIGINFO, signal_starter);
  signal(SIGUSR1, signal_starter);
  signal(SIGUSR2, signal_starter);} } // end unnamed namespace

int main(int argc, char *argv[]) {
  Argv_t external_command_line(&argv[0], &argv[argc], 0, Rwsh_stream_p());
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
