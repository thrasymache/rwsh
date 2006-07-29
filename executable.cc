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

bool Executable_t::increment_nesting(const Argv_t& argv) {
  if (excessive_nesting_v || global_nesting > max_nesting+1)
    return excessive_nesting_v = true;
  else {
    ++executable_nesting;
    ++global_nesting;
    return false;}}

bool Executable_t::decrement_nesting(const Argv_t& argv) {
  --global_nesting;
  if (excessive_nesting_v) excessive_nesting_handler(argv);
  --executable_nesting;
  return excessive_nesting_v;}

// code to call rwsh.excessive_nesting, separated out of operator() for clarity.
// The requirements for excessive nesting handling to work properly are as 
// follows: All things derived from Executable_t must call increment_nesting
// on start and decrement_nesting before terminating.  If they run more 
// than one other executable, then they must call excessive_nesting() in 
// between each executable, which must be of a Executable_t rather than a 
// direct call to the function that implements a builtin (the code below to 
// handle recursively excessive nesting is the only exception to this rule).
// main does not need to do this excessive_nesting handling, because anything 
// that it calls directly will have an initial nesting of 0.
void Executable_t::excessive_nesting_handler(const Argv_t& src_argv) {
  call_stack.push_back(src_argv[0]);
  if (in_excessive_nesting_handler) return;
  if (!global_nesting) {
    excessive_nesting_v = false;
    in_excessive_nesting_handler = true;
    Argv_t call_stack_copy = call_stack;                     //need for a copy: 
    call_stack.clear();
    call_stack.push_back("rwsh.excessive_nesting");
    executable_map[call_stack_copy](call_stack_copy);
    if (excessive_nesting_v) {
      echo_bi(Argv_t("%echo rwsh.excessive_nesting itself "
                     "exceeded MAX_NESTING:\n"));
      call_stack[0] = "%echo";
      echo_bi(call_stack);
      echo_bi(Argv_t("%echo \noriginal call stack:\n"));
      call_stack_copy[0] = "%echo";
      echo_bi(call_stack_copy);
      newline_bi(Argv_t("%newline"));
      call_stack.clear();
      call_stack.push_back("rwsh.excessive_nesting");
      excessive_nesting_v = false;}
    in_excessive_nesting_handler = false;
    set_var("IF_TEST", "");}}
// need for a copy: if rwsh.excessive_nesting exceeds MAX_NESTING itself
//     then it will unwind the stack and write to call_stack. To preserve the 
//     original call stack, we need a copy of call_stack to be the argument.

Binary_t::Binary_t(const std::string& impl) : implementation(impl) {}

// run the given binary
int Binary_t::operator() (const Argv_t& argv_i) {
  if (increment_nesting(argv_i)) return dollar_question;
  int ret;
  if (!fork()) {  
    Old_argv_t argv(argv_i);
    char **env = export_env();
    int ret = execve(implementation.c_str(), argv.argv(), env);
    Argv_t error_argv("rwsh.binary_not_found " + argv_i[0]); 
    executable_map[error_argv](error_argv);
    exit(ret);}
  else wait(&ret);
  dollar_question = last_return = ret;
  if (decrement_nesting(argv_i)) ret = dollar_question;
  if (del_on_term && !executable_nesting) delete this;
  return ret;}

Built_in_t::Built_in_t(const std::string& name_i, 
                       int (*impl)(const Argv_t& argv)) : 
  implementation(impl), name_v(name_i) {}

// run the given builtin
int Built_in_t::operator() (const Argv_t& argv) {
  if (increment_nesting(argv)) return dollar_question;
  dollar_question = last_return = (*implementation)(argv);
  int ret = last_return;
  if (decrement_nesting(argv)) ret = dollar_question;
  if (del_on_term && !executable_nesting) delete this;
  return ret;}

