// The definition of the Command_stream_t class. It is constructed from a 
// standard stream and defines an input operator for Argv_t objects. It also
// handles the calling of rwsh.prompt.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "argv.h"
#include "arg_script.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "variable_map.h"

// write the next command to dest. run rwsh.prompt as appropriate
Command_stream_t& Command_stream_t::operator>> (Argv_t& dest) {
  if (operator!()) return *this;
  std::string line;
  getline(src, line);
  if (operator!()) return *this;
  Argv_t raw_command("rwsh.raw_command");
  Executable_map_t::iterator e = executable_map.find(raw_command);
  if (e != executable_map.end()) {
    raw_command.push_back(line);
    (*e->second)(raw_command);
    if (Executable_t::unwind_stack()) return *this;}
  try {dest = Argv_t(line);}
  catch (Argv_t exception) {dest = exception;}
  return *this;}

// returns non-zero if the last command was read successfully
Command_stream_t::operator void* () const {
  if (exit_requested || Executable_t::unwind_stack()) return 0;
  else return src.operator void*();}

// returns true if the last command could not be read
bool Command_stream_t::operator! () const {
  return (exit_requested) || Executable_t::unwind_stack() || src.fail();} 

