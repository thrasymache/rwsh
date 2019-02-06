// The definition of the Base_executable and Binary classes. The former
// provides functionality common to all derived types, the latter executes
// external programs.
//
// Copyright (C) 2005-2019 Samuel Newbold

#include <list>
#include <map>
#include <set>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "call_stack.h"
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
#include "plumber.h"

void Base_executable::operator() (const Argm& argm,
                                 Error_list& parent_exceptions) {
  if (global_stack.global_nesting > global_stack.max_nesting+1)
    parent_exceptions.add_error(Exception(Argm::Excessive_nesting));
  if (global_stack.unwind_stack()) return;
  else ++executable_nesting, ++global_stack.global_nesting;
  Error_list children;
  struct timeval start_time;
  gettimeofday(&start_time, rwsh_clock.no_timezone);
  try {execute(argm, children);}
  catch (Exception error) {children.add_error(error);}
  struct timeval end_time;
  gettimeofday(&end_time, rwsh_clock.no_timezone);
  last_execution_time_v = Clock::timeval_sub(end_time, start_time);
  Clock::timeval_add(total_execution_time_v, last_execution_time_v);
  ++execution_count_v;
  --global_stack.global_nesting;
  if (global_stack.caught_signal) {
    Exception focus(global_stack.caught_signal);
    global_stack.caught_signal = Argm::No_exception;
    executable_map.run(focus, children);}
  if (children.size()) {
    last_exception_v = "";
    for (auto j = children.begin(); j != children.end();) {
      last_exception_v += j->str() + " " + argm.str();
      j->push_back(argm[0]);
      parent_exceptions.push_back(*j);
      if (++j != children.end()) last_exception_v += "; ";}}
  --executable_nesting;
  if (del_on_term && !executable_nesting) delete this;}

Binary::Binary(const std::string& impl) : implementation(impl) {}

// run the given binary
void Binary::execute(const Argm& argm_i, Error_list& exceptions) const {
  int ret,
      input = argm_i.input.fd(),
      output = argm_i.output.fd(),
      error = argm_i.error.fd();
  if (!fork()) {
    plumber.after_fork();
    if (dup2(input, 0) < 0) argm_i.error <<"dup2 didn't like changing input\n";
    if (dup2(output, 1) < 0)
      argm_i.error <<"dup2 didn't like changing output\n";
    if (dup2(error, 2) < 0) argm_i.error <<"dup2 didn't like changing error\n";
    Old_argv argv(argm_i.subrange(0));
    std::vector<char *>env;
    argm_i.export_env(env);
    ret = execve(implementation.c_str(), argv.argv(), &env[0]);
    exceptions.add_error(Exception(Argm::Binary_does_not_exist, argm_i[0]));
    global_stack.exception_handler(exceptions);
    executable_map.unused_var_check_at_exit();
    exit(ret);}
  else plumber.wait(&ret);
  if (WIFEXITED(ret) && WEXITSTATUS(ret))
    exceptions.add_error(Exception(Argm::Return_code, WEXITSTATUS(ret)));}
