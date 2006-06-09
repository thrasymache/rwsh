// The definition of the Binary_t and Built_in_t classes. The former executes
// external programs, the latter executes commands that are implemented by
// functions in builtin.cc.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <string>
#include <map>
#include <vector>

#include "argv.h"
#include "builtin.h"
#include "executable.h"
#include "executable_map.h"
#include "variable_map.h"

int Executable_t::global_nesting(0);
bool Executable_t::excessive_nesting_v(false);
bool Executable_t::in_excessive_nesting_handler(false);
Argv_t Executable_t::call_stack("rwsh.excessive_nesting");

bool Executable_t::excessive_nesting(const Argv_t& argv) {
  if (excessive_nesting_v) return decrement_nesting(argv);
  else return false;}

bool Executable_t::increment_nesting(const Argv_t& argv) {
  if (global_nesting > max_nesting) {
    excessive_nesting_handler(argv);
    return excessive_nesting_v = true;}
  else {
    ++global_nesting;
    return false;}}

bool Executable_t::decrement_nesting(const Argv_t& argv) {
  --global_nesting;
  if (excessive_nesting_v) excessive_nesting_handler(argv);
  return excessive_nesting_v;}

// code to call rwsh.excessive_nesting, separated out of operator() for clarity.
void Executable_t::excessive_nesting_handler(const Argv_t& src_argv) {
  if (global_nesting) {
    call_stack.push_back(src_argv[0]);}
  else {
    excessive_nesting_v = false;
    Argv_t call_stack_copy = call_stack;                     //need for a copy: 
    call_stack.clear();
    call_stack.push_back("rwsh.excessive_nesting");
    if (in_excessive_nesting_handler) {
      Argv_t blank;
      echo_bi(Argv_t("echo rwsh.excessive_nesting itself "
                     "exceeded MAX_NESTING:"));
      newline_bi(blank);
      echo_bi(call_stack_copy);
      newline_bi(blank);
      echo_bi(Argv_t("echo original call stack:"));
      newline_bi(blank);
      echo_bi(src_argv);
      newline_bi(blank);}
    else {
      in_excessive_nesting_handler = true;
      executable_map[call_stack_copy](call_stack_copy);
      in_excessive_nesting_handler = false;}}}
// need for a copy: if rwsh.excessive_nesting exceeds MAX_NESTING itself
//     then it will unwind the stack and write to call_stack. To preserve the 
//     original call stack, we need a copy of call_stack to be the argument.

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

