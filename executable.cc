// The definition of the Binary and Builtin classes. The former executes
// external programs, the latter executes commands that are implemented by
// functions in builtin.cc.
//
// Copyright (C) 2005-2017 Samuel Newbold

#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>
#include <set>
#include <string>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "builtin.h"
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
#include "plumber.h"

namespace {
Argm::Exception_t unix2rwsh(int sig) {
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
    default: return Argm::Sigunknown;}}
} // end unnamed namespace

int Base_executable::operator() (const Argm& argm,
                                 Error_list& parent_exceptions) {
  if (global_nesting > max_nesting+1)
    parent_exceptions.add_error(Exception(Argm::Excessive_nesting));
  if (unwind_stack()) return Variable_map::dollar_question;
  else ++executable_nesting, ++global_nesting;
  Error_list children;
  struct timeval start_time;
  gettimeofday(&start_time, rwsh_clock.no_timezone);
  int ret;
  try {ret = execute(argm, children);}
  catch (Exception error) {
    children.add_error(error);
    ret = -1;}
  struct timeval end_time;
  gettimeofday(&end_time, rwsh_clock.no_timezone);
  last_execution_time_v = Clock::timeval_sub(end_time, start_time);
  Clock::timeval_add(total_execution_time_v, last_execution_time_v);
  ++execution_count_v;
  Variable_map::dollar_question = last_return = ret;
  --global_nesting;
  if (caught_signal) {
    Exception focus(caught_signal);
    caught_signal = Argm::No_exception;
    executable_map.run(focus, children);}
  if (children.size()) {
    last_exception_v = "";
    for (Error_list::iterator i = children.begin(); i != children.end();) {
      last_exception_v += i->str() + " " + argm.str();
      i->push_back(argm[0]);
      parent_exceptions.push_back(*i);
      if (++i != children.end()) last_exception_v += "; ";}}
  --executable_nesting;
  if (del_on_term && !executable_nesting) delete this;
  return ret;}

void Base_executable::add_error(void) {
  unwind_stack_v = true;
  ++current_exception_count;}

void Error_list::add_error(const Argm& error){
  push_back(error);
  Base_executable::add_error();}

void Base_executable::unix_signal_handler(int sig) {
  caught_signal = unix2rwsh(sig);}

/* code to call exception handlers, separated out of operator() for clarity.
   The requirements for stack unwinding to work properly are as
   follows: any code that runs more than one other executable must test
   unwind_stack() in between each executable, which must be of an Executable
   rather than a direct call to the function that implements a builtin (the
   code below to run the fallback_handler is the only exception to this rule).
   main does not need to do this handling, because anything that it calls
   directly will have an initial nesting of 0.*/
void Base_executable::exception_handler(Error_list& exceptions) {
  in_exception_handler = true;
  unwind_stack_v = false;
  std::set<std::string> failed_handlers;
  for(;exceptions.size(); exceptions.pop_front(), --current_exception_count){
    Argm& focus(exceptions.front());
    if (failed_handlers.find(focus[0]) == failed_handlers.end()) {
      executable_map.run(focus, exceptions);
      if (unwind_stack()) {
        failed_handlers.insert(focus[0]);
        exceptions.insert(++exceptions.begin(), exceptions.back());
        exceptions.pop_back();
        unwind_stack_v = false;}}
    if (failed_handlers.find(focus[0]) != failed_handlers.end())
      b_fallback_handler(Argm(".fallback_handler", focus.begin(), focus.end(),
                              focus.argfunction(), Variable_map::global_map,
                              focus.input, focus.output, focus.error),
                         exceptions);}
  Variable_map::dollar_question = -1;
  dropped_catches = 0;
  collect_excess_thrown = execution_handler_excess_thrown = false;
  in_exception_handler = false;}

// code to call exception handlers when requested within a function
void Base_executable::catch_blocks(const Argm& argm,
                                   Error_list& exceptions) {
  for (Error_list::iterator focus = exceptions.begin();
       focus != exceptions.end();)
    if (find(argm.begin() + 1, argm.end(), (*focus)[0]) != argm.end()) {
      if (dropped_catches >= max_extra) {
         if (!execution_handler_excess_thrown)
           exceptions.add_error(
                     Exception(Argm::Excessive_exceptions_in_catch, max_extra));
         execution_handler_excess_thrown = true;
         return;}
      in_exception_handler = true;
      unwind_stack_v = false;
      unsigned previous_count = current_exception_count;
      //Error_list current_exceptions;
      executable_map.run(*focus, exceptions);
      dropped_catches += current_exception_count - previous_count;
      if (!unwind_stack()) {
        focus = exceptions.erase(focus);
        --current_exception_count;}
      else focus++;}
    else focus++;
  if (current_exception_count) unwind_stack_v = true;
  else unwind_stack_v = false;
  Variable_map::dollar_question = -1;
  in_exception_handler = false;}

Binary::Binary(const std::string& impl) : implementation(impl) {}

#include <iostream>
// run the given binary
int Binary::execute(const Argm& argm_i, Error_list& exceptions) const {
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
    ret = execve(implementation.c_str(), argv.argv(), env);
    Exception error_argm(Argm::Binary_not_found, argm_i[0]);
    executable_map.run(error_argm, exceptions);
    executable_map.unused_var_check_at_exit();
    exit(ret);}
  else plumber.wait(&ret);
  if (ret) exceptions.add_error(Exception(Argm::Return_code, ret));
  return ret;}

Builtin::Builtin(const std::string& name_i,
                 int (*impl)(const Argm& argm, Error_list& exceptions)) :
  implementation(impl), name_v(name_i) {}

// run the given builtin
int Builtin::execute(const Argm& argm, Error_list& exceptions) const {
  return (*implementation)(argm, exceptions);}
