// The definition of the Binary and Builtin classes. The former executes
// external programs, the latter executes commands that are implemented by
// functions in builtin.cc.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <map>
#include <string>
#include <signal.h>
#include <sys/time.h>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "builtin.h"
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
#include "plumber.h"
#include "variable_map.h"

bool Executable::increment_nesting(const Argv& argv) {
  if (global_nesting > argv.max_nesting()+1) caught_signal = SIGEXNEST;
  if (unwind_stack()) return true;
  else {
    ++executable_nesting;
    ++global_nesting;
    return false;}}

bool Executable::decrement_nesting(const Argv& argv) {
  --global_nesting;
  if (unwind_stack()) {
    call_stack.push_back(argv[0]);
    if (!global_nesting && !in_signal_handler) signal_handler();}
  --executable_nesting;
  if (del_on_term && !executable_nesting) delete this;
  return unwind_stack();}

// code to call rwsh.excessive_nesting, separated out of operator() for clarity.
// The requirements for stack unwinding to work properly are as 
// follows: All things derived from Executable must call increment_nesting
// on start and decrement_nesting before terminating.  If they run more 
// than one other executable, then they must call unwind_stack() in 
// between each executable, which must be of a Executable rather than a 
// direct call to the function that implements a builtin (the code below to 
// handle recursively excessive nesting is the only exception to this rule).
// main does not need to do this handling, because anything 
// that it calls directly will have an initial nesting of 0.
void Executable::signal_handler(void) {
  extern Variable_map* vars;
  Argv call_stack_copy;                                    //need for a copy: 
  switch (caught_signal) {
    case SIGSUB: call_stack_copy.push_back("rwsh.failed_substitution"); break;
    case SIGVAR: call_stack_copy.push_back("rwsh.undefined_variable"); break;
    case SIGEXNEST: call_stack_copy.push_back("rwsh.excessive_nesting"); break;
    case SIGHUP: call_stack_copy.push_back("rwsh.sighup"); break;
    case SIGINT: call_stack_copy.push_back("rwsh.sigint"); break;
    case SIGQUIT: call_stack_copy.push_back("rwsh.sigquit"); break;
    case SIGPIPE: call_stack_copy.push_back("rwsh.sigpipe"); break;
    case SIGTERM: call_stack_copy.push_back("rwsh.sigterm"); break;
    case SIGTSTP: call_stack_copy.push_back("rwsh.sigtstp"); break;
    case SIGCONT: call_stack_copy.push_back("rwsh.sigcont"); break;
    case SIGCHLD: call_stack_copy.push_back("rwsh.sigchld"); break;
    case SIGUSR1: call_stack_copy.push_back("rwsh.sigusr1"); break;
    case SIGUSR2: call_stack_copy.push_back("rwsh.sigusr2"); break;
    case SIGRWSH: break;
    default: 
      call_stack_copy.push_back(".echo");
      call_stack_copy.push_back("caught unknown signal in");}
  std::copy(call_stack.begin(), call_stack.end(), 
            std::back_inserter(call_stack_copy));
  call_stack = Argv();
  in_signal_handler = true;
  caught_signal = SIGNONE;
  vars->unset("IF_TEST");
  executable_map.run(call_stack_copy);
  if (unwind_stack()) {
    default_output <<"signal handler itself triggered signal\n";
    call_stack.push_front(".echo");
    b_echo(call_stack);
    default_output <<"\noriginal call stack:\n";
    call_stack_copy[0] = ".echo";
    b_echo(call_stack_copy);
    default_output <<"\n";
    default_output.flush();
    dollar_question = -1;
    call_stack = Argv();
    caught_signal = SIGNONE;
    vars->unset("IF_TEST");}
  in_signal_handler = false;}
// need for a copy: if the internal function that runs for this signal itself
//     triggers a signal then it will unwind the stack and write to call_stack. 
//     To preserve the original call stack, we need a copy of call_stack to be 
//     the argument.

Binary::Binary(const std::string& impl) : implementation(impl) {}

#include <iostream>
// run the given binary
int Binary::operator() (const Argv& argv_i) {
  try {
    if (increment_nesting(argv_i)) return dollar_question;
    struct timeval start_time;
    gettimeofday(&start_time, rwsh_clock.no_timezone);
    ++execution_count_v;
    int ret,
        input = argv_i.input.fd(),
        output = argv_i.output.fd(),
        error = argv_i.error.fd();
    if (!fork()) {  
      plumber.after_fork();
      if (dup2(input, 0) < 0) std::cerr <<"dup2 didn't like changing input\n";
      if (dup2(output, 1) < 0) std::cerr <<"dup2 didn't like changing output\n";
      if (dup2(error, 2) < 0) std::cerr <<"dup2 didn't like changing error\n";
      Old_argv argv(argv_i);
      char **env = argv_i.export_env();
      int ret = execve(implementation.c_str(), argv.argv(), env);
      Argv error_argv;
      error_argv.push_back("rwsh.binary_not_found");
      error_argv.push_back(argv_i[0]); 
      executable_map.run(error_argv);
      exit(ret);}
    else plumber.wait(&ret);
    dollar_question = last_return = ret;
    struct timeval end_time;
    gettimeofday(&end_time, rwsh_clock.no_timezone);
    last_execution_time_v = Clock::timeval_sub(end_time, start_time);
    Clock::timeval_add(total_execution_time_v, last_execution_time_v);
    if (decrement_nesting(argv_i)) ret = dollar_question;
    return ret;}
  catch (Signal_argv error) {
    caught_signal = SIGRWSH;
    std::copy(error.begin(), error.end(), std::back_inserter(call_stack));
    decrement_nesting(argv_i);
    return -1;}}

Builtin::Builtin(const std::string& name_i, 
                       int (*impl)(const Argv& argv)) : 
  implementation(impl), name_v(name_i) {}

// run the given builtin
int Builtin::operator() (const Argv& argv) {
  try {
    if (increment_nesting(argv)) return dollar_question;
    struct timeval start_time;
    gettimeofday(&start_time, rwsh_clock.no_timezone);
    ++execution_count_v;
    int ret = dollar_question = last_return = (*implementation)(argv);
    struct timeval end_time;
    gettimeofday(&end_time, rwsh_clock.no_timezone);
    last_execution_time_v = Clock::timeval_sub(end_time, start_time);
    Clock::timeval_add(total_execution_time_v, last_execution_time_v);
    if (decrement_nesting(argv)) ret = dollar_question;
    return ret;}
  catch (Signal_argv error) {
    caught_signal = SIGRWSH;
    std::copy(error.begin(), error.end(), std::back_inserter(call_stack));
    decrement_nesting(argv);
    return -1;}}

