// The main function for rwsh and the definition of rwsh.init
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <iostream>
#include <map>
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
  "%set RC $1;"
  "%set IF_TEST false;"
  "%elif /bin/test $1 {%source $RC; %exit};"
  "%elif %true {%source /etc/rwshrc}";}

int main(int argc, char *argv[]) {
  Argv_t external_command_line(&argv[0], &argv[argc], 0);
  Command_stream_t command_stream(std::cin);
  Argv_t command;
  Executable_map_t::iterator e;
  executable_map.set(new Function_t("rwsh.init", init_str));
  executable_map[Argv_t("rwsh.init")](external_command_line);
  while (command_stream >> command) {
    try {
      Arg_script_t script(command);
      command = script.interpret(command);}
    catch (Argv_t exception) {command = exception;}
    int run_command;
    e = executable_map.find(Argv_t("rwsh.before_command"));
    if (e != executable_map.end()) 
      run_command = (*e->second)(command);
    else run_command = 0;
    if (!run_command) executable_map[command](command);
    e = executable_map.find(Argv_t("rwsh.after_command"));
    if (e != executable_map.end()) (*e->second)(command);}
  e = executable_map.find(Argv_t("rwsh.shutdown"));
  if (e != executable_map.end()) 
    (*e->second)(external_command_line);
  return dollar_question;}
