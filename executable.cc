// The definition of the Binary_t and Built_in_t classes. The former executes
// external programs, the latter executes commands that are implemented by
// functions in builtin.cc.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <string>
#include <map>
#include <vector>

#include "argv.h"
#include "executable.h"
#include "executable_map.h"
#include "variable_map.h"

Binary_t::Binary_t(const std::string& impl) : implementation(impl) {}

// run the given binary
int Binary_t::operator() (const Argv_t& argv_i) {
  ++current_nesting;
  int ret;
  if (!fork()) {  
    Old_argv_t argv(argv_i);
    char **env = export_env();
    int ret = execve(implementation.c_str(), argv.argv(), env);
    Argv_t error_argv("rwsh.binary_not_found " + argv_i[0]); 
    executable_map[error_argv](error_argv);
    exit(ret);}
  else wait(&ret);
  --current_nesting;
  dollar_question = last_return = ret;
  if (del_on_term && !current_nesting) delete this;
  return ret;}

Built_in_t::Built_in_t(const std::string& name_i, 
                       int (*impl)(const Argv_t& argv)) : 
  implementation(impl), name_v(name_i) {}

// run the given builtin
int Built_in_t::operator() (const Argv_t& argv) {
  ++current_nesting;
  dollar_question = last_return = (*implementation)(argv);
  --current_nesting;
  int ret = last_return;
  if (del_on_term && !current_nesting) delete this;
  return ret;}

