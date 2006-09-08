// The main function for rwsh and the definition of rwsh.init
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <iostream>
#include <map>
#include <signal.h>
#include <string>
#include <vector>

#include "argv.h"
#include "arg_script.h"
#include "command_stream.h"
#include "selection.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "variable_map.h"

Executable_map_t executable_map;

namespace {std::string init_str =
  "%set MAX_NESTING 4;"
  "%set rc $2;"
  "%if %test_not_empty $rc {%source $rc};"
  "%else {%source /etc/rwshrc}";}

static void signal_starter(int sig) {Executable_t::caught_signal = sig;}

static void register_signals(void) {
  signal(SIGHUP, signal_starter);
  signal(SIGINT, signal_starter);
  signal(SIGQUIT, signal_starter);
  signal(SIGPIPE, signal_starter);
  signal(SIGTERM, signal_starter);
  signal(SIGTSTP, signal_starter);
  signal(SIGCONT, signal_starter);
  signal(SIGINFO, signal_starter);
  signal(SIGUSR1, signal_starter);
  signal(SIGUSR2, signal_starter);}

int main(int argc, char *argv[]) {
  Argv_t external_command_line(&argv[0], &argv[argc], 0);
  Command_stream_t command_stream(std::cin);
  executable_map.set(new Function_t("rwsh.init", init_str));
  external_command_line.push_front("rwsh.init");
  executable_map[external_command_line](external_command_line);
  register_signals();
  Argv_t command;
  Argv_t prompt("rwsh.prompt");
  Executable_map_t::iterator e = executable_map.find(prompt);
  if (command_stream && e != executable_map.end()) (*e->second)(prompt);
  while (command_stream >> command || Executable_t::unwind_stack()) 
    if (Executable_t::unwind_stack()) Executable_t::signal_handler();
    else {
      try {
        Arg_script_t script(command);
        command = script.interpret(command);}
      catch (Argv_t exception) {command = exception;}
      int run_command;
      command.push_front("rwsh.before_command");
      e = executable_map.find(command);
      if (e != executable_map.end()) 
        run_command = (*e->second)(command);
      else run_command = 0;
      command.pop_front();
      if (!run_command) executable_map[command](command);
      command.push_front("rwsh.after_command");
      e = executable_map.find(command);
      if (e != executable_map.end()) (*e->second)(command);
      if (command_stream) {
        Executable_map_t::iterator e = executable_map.find(prompt);
        if (e != executable_map.end()) (*e->second)(prompt);}}
  external_command_line[0] = "rwsh.shutdown";
  e = executable_map.find(external_command_line);
  if (e != executable_map.end()) 
    (*e->second)(external_command_line);
  return dollar_question;}
