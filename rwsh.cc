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
  "%set MAX_NESTING 5;"
  "%function \\ {%true $*};"
  "%function # {%true $*};"
  "%function cat {%selection_set SELECT $1; /bin/cat @$SELECT};"
  "%function cp {%selection_set SELECT $1; /bin/cp @$SELECT $2};"
  "%function echo {%echo $*; %newline};"
  "%function interactive {%importenv; %set SELECT $HOME; "
             "%function rwsh.autofunction {echo attempting to add $0 to "
                                    "executable map; %autofunction $0 \\$*}};"
  "%function exit {%exit};"
  "%function fn {%function $* {rwsh.argfunction}};"
  "%function grep {%selection_set SELECT $2; /usr/bin/grep $1 @$SELECT};"
  "%function ls {%selection_set SELECT $1; %ls @$SELECT};"
  "%function less {%selection_set SELECT $1; /usr/bin/less @$SELECT};"
  "%function printenv {%printenv $*};"
  "%function set {%set $*};"
  "%function st {/bin/bash /usr/local/bin/test_rwsh};"
  "%function rwsh {/usr/local/bin/rwsh $*};"
  "%function rwsh.arguments_for_argfunction {echo cannot specify arguments "
             "for $1 directly; %false};"
  "%function rwsh.binary_not_found {echo $1 : could not run binary; %false};"
  "%function rwsh.excessive_nesting {echo function exceeded MAX_NESTING "
             "( $MAX_NESTING ). call stack: $*};"
  "%function rwsh.mismatched_brace {echo mismatched brace: $1; %false};"
  "%function rwsh.multiple_argfunctions {echo multiple argfunctions not "
             "supported at this time; %false};"
  "%function rwsh.prompt {%echo $? \\$};"
  "%function rwsh.selection_not_found{echo no file matching pattern $1 for "
             "selection $2};"
  "%function rwsh.shutdown {%newline};"
  "%function vi {%selection_set SELECT $1; /usr/bin/vi @$SELECT};"
  "%function which {%which $* {rwsh.argfunction}};"
  "%version";}

int main(int argc, char *argv[]) {
  Argv_t external_command_line(&argv[0], &argv[argc], 0);
  Command_stream_t command_stream(std::cin);
  Argv_t command;
  Executable_map_t::iterator executable;
  executable_map.set(new Function_t("rwsh.init", init_str));
  executable_map[Argv_t("rwsh.init")](external_command_line);
  while (command_stream >> command) {
    int run_command;
    executable = executable_map.find(Argv_t("rwsh.before_command"));
    if (executable != executable_map.end()) 
      run_command = (*executable->second)(command);
    else run_command = 0;
    if (!run_command) executable_map[command](command);
    executable = executable_map.find(Argv_t("rwsh.after_command"));
    if (executable != executable_map.end()) (*executable->second)(command);}
  executable = executable_map.find(Argv_t("rwsh.shutdown"));
  if (executable != executable_map.end()) 
    (*executable->second)(external_command_line);
  return dollar_question;}
