// The definition of the Binary and Builtin classes. The former executes
// external programs, the latter executes commands that are implemented by
// functions in builtin.cc.
//
// Copyright (C) 2005-2015 Samuel Newbold

#include <map>
#include <string>
#include <signal.h>
#include <sys/time.h>
#include <vector>

#include "rwsh_stream.h"

#include "argm.h"
#include "builtin.h"
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
#include "plumber.h"
#include "variable_map.h"

namespace {
Argm::Sig_type unix2rwsh(int sig) {
  switch (sig) {
    case SIGHUP: return Argm::Sighup;
    case SIGINT: return Argm::Sigint;
    case SIGQUIT: return Argm::Sigquit;
    case SIGPIPE: return Argm::Sigpipe;
    case SIGTERM: return Argm::Sigterm;
    case SIGTSTP: return Argm::Sigtstp;
    case SIGCONT: return Argm::Sigcont;
    case SIGCHLD: return Argm::Sigchld;
    case SIGUSR1: return Argm::Sigusr1;
    case SIGUSR2: return Argm::Sigusr2;
    default: return Argm::Sigunknown;}}}

bool Executable::increment_nesting(const Argm& argm) {
  if (global_nesting > argm.max_nesting()+1) {
    //throw Signal_argm(Argm::Excessive_nesting);}
    caught_signal = Argm::Excessive_nesting;
    call_stack.push_back(Argm::signal_names[Argm::Excessive_nesting]);}
  if (unwind_stack()) return true;
  else {
    ++executable_nesting;
    ++global_nesting;
    return false;}}

bool Executable::decrement_nesting(const Argm& argm) {
  --global_nesting;
  if (unwind_stack()) {
    call_stack.push_back(argm[0]);
    if (!global_nesting && !in_signal_handler) signal_handler();}
  --executable_nesting;
  if (del_on_term && !executable_nesting) delete this;
  return unwind_stack();}

void Executable::unix_signal_handler(int sig) {
  caught_signal = unix2rwsh(sig);
  call_stack.push_back(Argm::signal_names[caught_signal]);}

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
  Argm call_stack_copy(call_stack);                          //need for a copy:
  call_stack = Argm(default_input, default_output, default_error);
  in_signal_handler = true;
  caught_signal = Argm::No_signal;
  Variable_map::global_map->unset("IF_TEST");
  executable_map.run(call_stack_copy);
  if (unwind_stack()) {
    default_output <<"signal handler itself triggered signal\n";
    Argm temp_call_stack(".echo", call_stack.begin(), call_stack.end(),
                         call_stack.argfunction(),
                         call_stack.input, call_stack.output, call_stack.error);
    call_stack = Argm(default_input, default_output, default_error);
    b_echo(temp_call_stack);
    default_output <<"\noriginal call stack:\n";
    call_stack_copy[0] = ".echo";
    b_echo(call_stack_copy);
    default_output <<"\n";
    default_output.flush();
    dollar_question = -1;
    caught_signal = Argm::No_signal;
    Variable_map::global_map->unset("IF_TEST");}
  in_signal_handler = false;}
// need for a copy: if the internal function that runs for this signal itself
//     triggers a signal then it will unwind the stack and write to call_stack. 
//     To preserve the original call stack, we need a copy of call_stack to be 
//     the argument.

Binary::Binary(const std::string& impl) : implementation(impl) {}

#include <iostream>
// run the given binary
int Binary::operator() (const Argm& argm_i) {
  try {
    if (increment_nesting(argm_i)) return dollar_question;
    struct timeval start_time;
    gettimeofday(&start_time, rwsh_clock.no_timezone);
    ++execution_count_v;
    int ret,
        input = argm_i.input.fd(),
        output = argm_i.output.fd(),
        error = argm_i.error.fd();
    if (!fork()) {  
      plumber.after_fork();
      if (dup2(input, 0) < 0) std::cerr <<"dup2 didn't like changing input\n";
      if (dup2(output, 1) < 0) std::cerr <<"dup2 didn't like changing output\n";
      if (dup2(error, 2) < 0) std::cerr <<"dup2 didn't like changing error\n";
      Old_argv argv(argm_i);
      char **env = argm_i.export_env();
      int ret = execve(implementation.c_str(), argv.argv(), env);
      Signal_argm error_argm(Argm::Binary_not_found, argm_i[0]); 
      executable_map.run(error_argm);
      exit(ret);}
    else plumber.wait(&ret);
    dollar_question = last_return = ret;
    struct timeval end_time;
    gettimeofday(&end_time, rwsh_clock.no_timezone);
    last_execution_time_v = Clock::timeval_sub(end_time, start_time);
    Clock::timeval_add(total_execution_time_v, last_execution_time_v);
    if (decrement_nesting(argm_i)) ret = dollar_question;
    return ret;}
  catch (Signal_argm error) {
    caught_signal = error.signal;
    std::copy(error.begin(), error.end(), std::back_inserter(call_stack));
    decrement_nesting(argm_i);
    return -1;}}

Builtin::Builtin(const std::string& name_i, 
                       int (*impl)(const Argm& argm)) : 
  implementation(impl), name_v(name_i) {}

// run the given builtin
int Builtin::operator() (const Argm& argm) {
  try {
    if (increment_nesting(argm)) return dollar_question;
    struct timeval start_time;
    gettimeofday(&start_time, rwsh_clock.no_timezone);
    ++execution_count_v;
    int ret = dollar_question = last_return = (*implementation)(argm);
    struct timeval end_time;
    gettimeofday(&end_time, rwsh_clock.no_timezone);
    last_execution_time_v = Clock::timeval_sub(end_time, start_time);
    Clock::timeval_add(total_execution_time_v, last_execution_time_v);
    if (decrement_nesting(argm)) ret = dollar_question;
    return ret;}
  catch (Signal_argm error) {
    caught_signal = error.signal;
    std::copy(error.begin(), error.end(), std::back_inserter(call_stack));
    decrement_nesting(argm);
    return -1;}}

