// The definition of the Binary_t and Built_in_t classes. The former executes
// external programs, the latter executes commands that are implemented by
// functions in builtin.cc.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <string>
#include <signal.h>
#include <map>
#include <vector>

#include "argv.h"
#include "builtin.h"
#include "executable.h"
#include "executable_map.h"
#include "variable_map.h"

int Executable_t::global_nesting(0);
int Executable_t::caught_signal(0);
bool Executable_t::in_signal_handler(false);
Argv_t Executable_t::call_stack;

bool Executable_t::increment_nesting(const Argv_t& argv) {
  if (global_nesting > max_nesting+1) caught_signal = SIGEXNEST;
  if (unwind_stack()) return true;
  else {
    ++executable_nesting;
    ++global_nesting;
    return false;}}

bool Executable_t::decrement_nesting(const Argv_t& argv) {
  --global_nesting;
  if (unwind_stack()) {
    call_stack.push_back(argv[0]);
    if (!global_nesting && !in_signal_handler) signal_handler();}
  --executable_nesting;
  return unwind_stack();}

// code to call rwsh.excessive_nesting, separated out of operator() for clarity.
// The requirements for stack unwinding to work properly are as 
// follows: All things derived from Executable_t must call increment_nesting
// on start and decrement_nesting before terminating.  If they run more 
// than one other executable, then they must call unwind_stack() in 
// between each executable, which must be of a Executable_t rather than a 
// direct call to the function that implements a builtin (the code below to 
// handle recursively excessive nesting is the only exception to this rule).
// main does not need to do this handling, because anything 
// that it calls directly will have an initial nesting of 0.
void Executable_t::signal_handler(void) {
  Argv_t call_stack_copy = call_stack;                     //need for a copy: 
  switch (caught_signal) {
    case SIGEXNEST: call_stack_copy.push_front("rwsh.excessive_nesting"); break;
    case SIGHUP: call_stack_copy.push_front("rwsh.sighup"); break;
    case SIGINT: call_stack_copy.push_front("rwsh.sigint"); break;
    case SIGQUIT: call_stack_copy.push_front("rwsh.sigquit"); break;
    case SIGPIPE: call_stack_copy.push_front("rwsh.sigpipe"); break;
    case SIGTERM: call_stack_copy.push_front("rwsh.sigterm"); break;
    case SIGTSTP: call_stack_copy.push_front("rwsh.sigtstp"); break;
    case SIGCONT: call_stack_copy.push_front("rwsh.sigcont"); break;
    case SIGCHLD: call_stack_copy.push_front("rwsh.sigchld"); break;
    case SIGINFO: call_stack_copy.push_front("rwsh.siginfo"); break;
    case SIGUSR1: call_stack_copy.push_front("rwsh.sigusr1"); break;
    case SIGUSR2: call_stack_copy.push_front("rwsh.sigusr2"); break;
    default: 
      call_stack_copy.push_front("caught unknown signal in");
      call_stack_copy.push_front("%echo");}
  call_stack.clear();
  in_signal_handler = true;
  caught_signal = SIGNONE;
  set_var("IF_TEST", "");
  executable_map[call_stack_copy](call_stack_copy);
  if (unwind_stack()) {
    echo_bi(Argv_t("%echo signal handler itself triggered signal\n"));
    call_stack.push_front("%echo");
    echo_bi(call_stack);
    echo_bi(Argv_t("%echo \noriginal call stack:\n"));
    call_stack_copy[0] = "%echo";
    echo_bi(call_stack_copy);
    newline_bi(Argv_t("%newline"));
    return_bi(Argv_t("%return -1"));
    call_stack.clear();
    caught_signal = SIGNONE;
    set_var("IF_TEST", "");}
  in_signal_handler = false;}
// need for a copy: if the internal function that runs for this signal itself
//     triggers a signal then it will unwind the stack and write to call_stack. 
//     To preserve the original call stack, we need a copy of call_stack to be 
//     the argument.

Binary_t::Binary_t(const std::string& impl) : implementation(impl) {}

// run the given binary
int Binary_t::operator() (const Argv_t& argv_i) {
  if (increment_nesting(argv_i)) return dollar_question;
  int ret;
  if (!fork()) {  
    Old_argv_t argv(argv_i);
    char **env = export_env();
    int ret = execve(implementation.c_str(), argv.argv(), env);
    Argv_t error_argv;
    error_argv.push_back("rwsh.binary_not_found");
    error_argv.push_back(argv_i[0]); 
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

