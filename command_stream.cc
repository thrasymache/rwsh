// The definition of the Command_stream_t class. It is constructed from a 
// standard stream and defines an input operator for Argv_t objects. It also
// handles the calling of rwsh.prompt.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "variable_map.h"

struct timeval Command_stream_t::waiting_for_binary_v = {0, 0};
struct timeval Command_stream_t::waiting_for_shell_v = {0, 0};
struct timeval Command_stream_t::waiting_for_user_v = {0, 0};

Command_stream_t::Command_stream_t(std::istream& s) : src(s) {
  gettimeofday(&after_input, &Executable_t::no_timezone);}

// write the next command to dest. run rwsh.prompt as appropriate
Command_stream_t& Command_stream_t::operator>> (Arg_script_t& dest) {
  if (operator!()) return *this;
  std::string line;
  struct timeval old_after_input = after_input;
  gettimeofday(&before_input, &Executable_t::no_timezone);
  getline(src, line);
  gettimeofday(&after_input, &Executable_t::no_timezone);
  Executable_t::timeval_add(waiting_for_shell_v,
                      Executable_t::timeval_sub(before_input, old_after_input));
  Executable_t::timeval_add(waiting_for_user_v,
                      Executable_t::timeval_sub(after_input, before_input));
  if (operator!()) return *this;
  Argv_t raw_command;
  raw_command.push_back(line);
  executable_map.run_if_exists("rwsh.raw_command", raw_command);
  if (Executable_t::unwind_stack()) return *this;
  dest = Arg_script_t(line, 0);
  return *this;}

// returns non-zero if the last command was read successfully
Command_stream_t::operator void* () const {
  if (Variable_map_t::exit_requested) return 0;
  else return src.operator void*();}

// returns true if the last command could not be read
bool Command_stream_t::operator! () const {
  return Variable_map_t::exit_requested || src.fail();} 

