// The definition of the Command_stream_t class. It is constructed from a 
// standard stream and defines an input operator for Argv_t objects. It also
// handles the calling of rwsh.prompt.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "clock.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "variable_map.h"

Command_stream_t::Command_stream_t(std::istream& s, bool subprompt_i) :
    src(s), subprompt(subprompt_i) {}

// write the next command to dest. run rwsh.prompt as appropriate
Command_stream_t& Command_stream_t::operator>> (Arg_script_t& dest) {
  if (operator!()) return *this;
  std::string cmd;
  for (bool cmd_is_incomplete=true; cmd_is_incomplete;) {
    std::string line;
    struct timeval before_input, after_input;
    gettimeofday(&before_input, rwsh_clock.no_timezone);
    getline(src, line);
    gettimeofday(&after_input, rwsh_clock.no_timezone);
    rwsh_clock.user_wait(before_input, after_input);
    if (operator!()) return *this;
    cmd += line;
    try {
      dest = Arg_script_t(cmd, 0);
      cmd_is_incomplete = false;}
    catch (Unclosed_parenthesis_t exception) {
      cmd += '\n';}
    catch (Unclosed_brace_t exception) {
      cmd += '\n';}
    catch (...) {
      Argv_t raw_command;
      raw_command.push_back(cmd);
      executable_map.run_if_exists("rwsh.raw_command", raw_command);
      throw;}}
  Argv_t raw_command;
  raw_command.push_back(cmd);
  executable_map.run_if_exists("rwsh.raw_command", raw_command);
  if (Executable_t::unwind_stack()) return *this;
  return *this;}

// returns non-zero if the last command was read successfully
Command_stream_t::operator void* () const {
  if (Variable_map_t::exit_requested) return 0;
  else return src.operator void*();}

// returns true if the last command could not be read
bool Command_stream_t::operator! () const {
  return Variable_map_t::exit_requested || src.fail();} 

