// The definition of the Command_stream class. It is constructed from a
// standard stream and defines an input operator for Argm objects. It also
// handles the calling of .prompt.
//
// Copyright (C) 2005-2018 Samuel Newbold

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <set>
#include <sstream>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "call_stack.h"
#include "clock.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

Command_stream::Command_stream(Rwsh_istream_p& s, bool subprompt_i) :
    src(s), subprompt(subprompt_i) {}

// write the next command to dest. run .prompt as appropriate
Command_stream& Command_stream::getline(Arg_script& dest, Error_list& errors) {
  if (this->fail()) return *this;
  std::string cmd;
  for (bool cmd_is_incomplete=true; cmd_is_incomplete;) {
    errors.reset(); // If this is the first execution of the loop, then errors
    // should be empty, if a subsequent execution, then we will regenerate all
    // the errors again. Obviously this should go away when this gets fixed to
    // be single-pass.
    std::string line;
    struct timeval before_input, after_input;
    gettimeofday(&before_input, rwsh_clock.no_timezone);
    src.getline(line);
    gettimeofday(&after_input, rwsh_clock.no_timezone);
    rwsh_clock.user_wait(before_input, after_input);
    if (fail())
      if (cmd.size())  {                 // EOF without a complete command
        Exception raw_command(Argm::Raw_command, cmd);
        executable_map.run_handling_exceptions(raw_command, errors);
        Arg_script(cmd, 0, errors);}     // this will throw the right exception
      else return *this;
    else cmd += line;
    try {
      dest = Arg_script(cmd, 0, errors);
      if (global_stack.unwind_stack()) {
        Exception raw_command(Argm::Raw_command, cmd);
        global_stack.catch_one(raw_command, errors);
        return *this;}
      cmd_is_incomplete = false;}
    catch (Unclosed_parenthesis exception) {
      cmd += '\n';}
    catch (Unclosed_brace exception) {
      cmd += '\n';}}
  Exception raw_command(Argm::Raw_command, cmd);
  executable_map.run_handling_exceptions(raw_command, errors);
  return *this;}

// returns true if the last command could not be read
bool Command_stream::fail() const {
  return global_stack.exit_requested || src.fail();}
