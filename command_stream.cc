// The definition of the Command_stream class. It is constructed from a
// standard stream and defines an input operator for Argm objects. It also
// handles the calling of rwsh.prompt.
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
#include "clock.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

Command_stream::Command_stream(Rwsh_istream_p& s, bool subprompt_i) :
    src(s), subprompt(subprompt_i) {}

// write the next command to dest. run rwsh.prompt as appropriate
Command_stream& Command_stream::operator>> (Arg_script& dest) {
  if (this->fail()) return *this;
  std::string cmd;
  for (bool cmd_is_incomplete=true; cmd_is_incomplete;) {
    std::string line;
    struct timeval before_input, after_input;
    gettimeofday(&before_input, rwsh_clock.no_timezone);
    src.getline(line);
    gettimeofday(&after_input, rwsh_clock.no_timezone);
    rwsh_clock.user_wait(before_input, after_input);
    if (fail())
      if (cmd.size())  {                 // EOF without a complete command
        Exception raw_command(Argm::Raw_command, cmd);
        executable_map.base_run(raw_command);
        Arg_script(cmd, 0);}             // this will throw the right exception
      else return *this;
    else cmd += line;
    try {
      dest = Arg_script(cmd, 0);
      cmd_is_incomplete = false;}
    catch (Unclosed_parenthesis exception) {
      cmd += '\n';}
    catch (Unclosed_brace exception) {
      cmd += '\n';}
    catch (...) {
      Exception raw_command(Argm::Raw_command, cmd);
      executable_map.base_run(raw_command);
      throw;}}
  Exception raw_command(Argm::Raw_command, cmd);
  executable_map.base_run(raw_command);
  return *this;}

// returns true if the last command could not be read
bool Command_stream::fail() const {
  return Variable_map::exit_requested || src.fail();}
