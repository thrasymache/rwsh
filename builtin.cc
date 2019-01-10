// The functions that implement each of the builtin executables
//
// Copyright (C) 2006-2018 Samuel Newbold

#include <algorithm>
#include <climits>
#include <cfloat>
#include <cstdlib>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <list>
#include <limits>
#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
extern char** environ;

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "builtin.h"
#include "call_stack.h"
#include "clock.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "file_stream.h"
#include "pipe_stream.h"
#include "plumber.h"
#include "prototype.h"
#include "read_dir.cc"
#include "rwshlib.h"
#include "selection.h"
#include "substitution_stream.h"
#include "tokenize.cc"

#include "function.h"

namespace {
std::string fallback_message = "Exception for failed handler. "
                               "Original exception with call stack:\n";
} // end unnamed namespace

// print the number of arguments passed
int b_argc(const Argm& argm, Error_list& exceptions) {
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<argm.argc()-1;
  return 0;}

// change the current directory to the one given
int b_cd(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  errno = 0;
  if (!chdir(argm[1].c_str()));
  else if (errno == ENOENT)
    exceptions.add_error(Exception(Argm::Directory_not_found, argm[1]));
  else if (errno == ENOTDIR)
    exceptions.add_error(Exception(Argm::Not_a_directory, argm[1]));
  // not tested. time for additional errors to be differentiated
  else exceptions.add_error(Exception(Argm::Internal_error, errno));
  return errno = 0;}

// run the argument function, collecting exceptions to be thrown as a group
// at the end, but terminating immediately if one of the specified exceptions
// are thrown
int b_collect_errors_except(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2)
    exceptions.add_error(Exception(Argm::Bad_argc, argm.argc()-1, 1, 0));
  if (!argm.argfunction())
    exceptions.add_error(Exception(Argm::Missing_argfunction));
  if (global_stack.unwind_stack()) return 0;
  else return global_stack.collect_errors_core(argm, true, exceptions);}

// run the argument function, collecting exceptions to be thrown as a group
// at the end, but only until an exception is not one of the specified
// exceptions
int b_collect_errors_only(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2)
    exceptions.add_error(Exception(Argm::Bad_argc, argm.argc()-1, 1, 0));
  if (!argm.argfunction())
    exceptions.add_error(Exception(Argm::Missing_argfunction));
  if (global_stack.unwind_stack()) return 0;
  else return global_stack.collect_errors_core(argm, false, exceptions);}

// echo arguments to standard output without space separation
int b_combine(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (auto i: argm.subrange(1)) argm.output <<i;
  argm.output.flush();
  return 0;}

// disable readline regardless of status
int b_disable_readline(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  readline_enabled = false;
  return 0;}

// echo arguments to standard output separated by space
int b_echo(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (auto i: argm.subrange(1, 1)) argm.output <<i <<" ";
  argm.output <<argm.back();
  argm.output.flush();
  return 0;}

// enable readline regardless of status
int b_enable_readline(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  readline_enabled = true;
  return 0;}

// echo arguments to standard error separated by space
int b_error(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (auto i: argm.subrange(1, 1)) argm.error <<i <<" ";
  argm.error <<argm.back();
  argm.error.flush();
  return 0;}

// replace the shell with the given binary
int b_exec(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  int input = argm.input.fd(),
      output = argm.output.fd(),
      error = argm.error.fd();
  if (dup2(input, 0) < 0) argm.error <<"dup2 didn't like changing input\n";
  if (dup2(output, 1) < 0) argm.error <<"dup2 didn't like changing output\n";
  if (dup2(error, 2) < 0) argm.error <<"dup2 didn't like changing error\n";
  Old_argv old_argv(argm.subrange(1));
  std::vector<char *>env;
  argm.export_env(env);
  execve(*old_argv.argv(), old_argv.argv(), &env[0]);
  // execve does not return on success, so what's the error?
  if (errno == ENOENT)
    exceptions.add_error(Exception(Argm::Binary_does_not_exist, argm[1]));
  else if (errno == EACCES || errno == ENOEXEC)
    exceptions.add_error(Exception(Argm::Not_executable, argm[1]));
  else exceptions.add_error(Exception(Argm::Exec_failed, argm[1], errno));
  return 0;}   // we are depending on the error handlers to indicate failure

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_execution_count(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->execution_count();
    argm.output.flush();
    return 0;}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// exit the shell with the specified exit value
int b_exit(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    global_stack.request_exit(my_strtoi(argm[1]));
    if (gc_state.in_if_block) gc_state.exception_thrown = true;}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}
  return 0;}

/* Exception handler for exceptions that trigger exceptions in their exception
   handler, without possibly itself triggering exceptions. This prints its
   arguments prefixed by a configurable message.*/
int b_fallback_handler(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  argm.output <<fallback_message;
  for (auto i: argm.subrange(1, 1)) argm.output <<i <<" ";
  argm.output <<argm.back() <<"\n";
  argm.output.flush();
#if 0
    argm.error <<*i <<" ";
  argm.error <<argm.back() <<"\n";
  argm.error.flush();
#endif
  return 0;}

// run the argfunction for each argument, passing that value as the argument
// returns the value returned by the argfunction
int b_for(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  int ret = 0;
  Argm body(argm.parent_map(), argm.input, argm.output, argm.error);
  body.push_back(".mapped_argfunction");
  body.push_back("");
  for (auto i: argm.subrange(1)) {
    if (argm.argfunction()) {
      body[1] = i;
      ret  = (*argm.argfunction())(body, exceptions);
      (void) global_stack.remove_exceptions(".continue", exceptions);
      if (global_stack.remove_exceptions(".break", exceptions) ||
          global_stack.unwind_stack()) return 0;}
    else ret = 0;}
  return ret;}

// run the argfunction for line of input, passing that line as the argm
// returns the value returned by the argfunction
int b_for_each_line(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  int ret = 0;
  while(!argm.input.fail()) {
    std::string line;
    // shouldn't interfere with input being consumed by this builtin
    Argm body(argm.parent_map(), default_input, argm.output, argm.error);
    body.push_back(".mapped_argfunction");
    argm.input.getline(line);
    if (argm.input.fail() && !line.size()) break;
    tokenize(line, std::back_inserter(body),
             std::bind2nd(std::equal_to<char>(), ' '));
    ret = (*argm.argfunction())(body, exceptions);
    (void) global_stack.remove_exceptions(".continue", exceptions);
    if (global_stack.remove_exceptions(".break", exceptions) ||
        global_stack.unwind_stack()) return 0;}
  return ret;}

int b_fork(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  // argfunction optional
  int status = 0;
  if (!fork()) {
    plumber.after_fork();
    Argm lookup(argm.subrange(1), argm.argfunction(),
                argm.parent_map(),
                argm.input, argm.output.child_stream(), argm.error);
    status = executable_map.run(lookup, exceptions);
    if (global_stack.unwind_stack()) {
      global_stack.exception_handler(exceptions);
      if (!global_stack.exit_value()) global_stack.request_exit(-1);}
    executable_map.unused_var_check_at_exit();
    std::exit(global_stack.exit_value());}
  else plumber.wait(&status);
  if (WIFEXITED(status) && WEXITSTATUS(status))
    exceptions.add_error(Exception(Argm::Return_code, WEXITSTATUS(status)));
  return 0;}

// add binary to executable map with name $1
int b_binary(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2)
    exceptions.add_error(Exception(Argm::Bad_argc, argm.argc()-1, 1, 0));
  if (argm.argfunction())
    exceptions.add_error(Exception(Argm::Excess_argfunction));
  if (global_stack.unwind_stack()) return 0;
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map());
  struct stat sb;
  if (stat(lookup[0].c_str(), &sb))
    exceptions.add_error(Exception(Argm::Binary_does_not_exist, lookup[0]));
  else if (!executable_map.find_second(lookup))
    executable_map.set(new Binary(lookup[0]));
  else exceptions.add_error(Exception(Argm::Executable_already_exists,
                                      lookup[0]));
  return 0;}

// add argfunction to executable map with name $1
int b_function(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map());
  Base_executable *e = executable_map.find_second(lookup);
  if (is_argfunction_name(argm[1]) || dynamic_cast<Builtin*>(e))
    exceptions.add_error(Exception(Argm::Illegal_function_name, argm[1]));
  else {
      argm.error <<"deprecated non-prototype: " <<argm.str() <<"\n";
      executable_map.set(new Function(argm[1],
                       argm.begin()+2, argm.end(), true, *argm.argfunction()));
  }
  return 0;}

// add argfunction to executable map with name $1 and arguments $*2
// the arguments must include all flags that can be passed to this function
int b_function_all_flags(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm lookup(argm.subrange(1, argm.argc()-2), nullptr, argm.parent_map());
  Base_executable *e = executable_map.find_second(lookup);
  if (is_argfunction_name(argm[1]) || dynamic_cast<Builtin*>(e))
    exceptions.add_error(Exception(Argm::Illegal_function_name, argm[1]));
  else executable_map.set(new Function(argm[1], argm.begin()+2, argm.end(),
                          false, *argm.argfunction()));
  return 0;}

// Get the configurable message for fallback_handler
int b_get_fallback_message(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<fallback_message;
  return 0;}

// Get the number of exceptions that can be thrown inside .collect_errors_*
// before they exit early
int b_get_max_collectible_exceptions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<global_stack.max_collect;
  return 0;}

// Get the number of exceptions that can be thrown by catch blocks after
// max_collectible_exceptions have already been thrown
int b_get_max_extra_exceptions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<global_stack.max_extra;
  return 0;}

// Get the maximum number of nesting levels where functions call functions
// before completing.
int b_get_max_nesting(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<global_stack.max_nesting;
  return 0;}

// add a variable to the variable map that will remain after the enclosing
// function terminates
int b_global(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.global(argm[1], argm[2]);
  return 0;}

namespace {
int if_core(const Argm& argm, Error_list& exceptions,
            Conditional_state& state, bool logic, bool is_else) {
  if (!state.in_if_block) throw Exception(Argm::Else_without_if);
  else if (state.successful_condition) return 0;
  else {
    int ret = 0;
    Argm lookup(argm.subrange(1), nullptr, argm.parent_map(),
                  argm.input, argm.output.child_stream(), argm.error);
    state.in_if_block = false;
    bool run = true;
    if (!is_else) {
      run = logic == executable_map.run_condition(lookup, exceptions);
      if (global_stack.unwind_stack())
        run = false, state.exception_thrown = true;
      else if (state.in_if_block) {
        state.in_if_block = false;
        // if only .false was thrown, then ignore any .if without .else
        if (logic == run) throw Exception(Argm::Bad_if_nest);}}
    if (run) {
      if (argm.argfunction()) {
        Argm mapped_argm(argm.parent_map(),
                         argm.input, argm.output.child_stream(), argm.error);
        mapped_argm.push_back(".mapped_argfunction");
        ret = (*argm.argfunction())(mapped_argm, exceptions);}
      if (global_stack.unwind_stack())
        state.exception_thrown = ! is_else;
      else if (state.in_if_block) {
        state.in_if_block = false;
        throw Exception(Argm::Bad_if_nest);}
      else state.in_if_block = state.successful_condition = true;}
    state.in_if_block = true;
    return ret;}}
}

// run argfunction if $* returns true
// returns the value returned by the argfunction
int b_if(const Argm& argm, Error_list& exceptions) {
  try {
    if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (gc_state.exception_thrown)
      gc_state.successful_condition = gc_state.exception_thrown = false;
    else if (gc_state.in_if_block) throw Exception(Argm::If_before_else);
    gc_state.in_if_block = true;
    return if_core(argm, exceptions, gc_state, true, false);}
  catch (Exception exception) {
    gc_state.exception_thrown = true;
    throw exception;}}

// run argfunction if successful_condition is false and $* returns true
// returns the value returned by the argfunction
int b_else_if(const Argm& argm, Error_list& exceptions) {
  try {
    if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (gc_state.exception_thrown) return 0;
    else return if_core(argm, exceptions, gc_state, true, false);}
  catch (Exception exception) {
    gc_state.exception_thrown = true;
    throw exception;}}

// run argfunction if successful_condition is false and $* returns false
// returns the value returned by the argfunction
int b_else_if_not(const Argm& argm, Error_list& exceptions) {
  try {
    if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (gc_state.exception_thrown) return 0;
    else return if_core(argm, exceptions, gc_state, false, false);}
  catch (Exception exception) {
    gc_state.exception_thrown = true;
    throw exception;}}

// run argfunction if successful_condition is false
// returns the value returned by the argfunction
int b_else(const Argm& argm, Error_list& exceptions) {
  try {
    int ret = 0;
    if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (gc_state.exception_thrown) gc_state.exception_thrown = false;
    else ret = if_core(argm, exceptions, gc_state, false, true);
    gc_state.successful_condition = gc_state.in_if_block = false;
    return ret;}
  catch (Exception exception) {
    gc_state.successful_condition = gc_state.in_if_block = false;
    throw exception;}}

// prints a list of all internal functions
int b_internal_functions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (int i = 1; i < Argm::Exception_count; ++i)
    argm.output <<Argm::exception_names[i] <<"\n";
  return 0;}

// returns one if the input stream is not the default_stream
int b_is_default_input(const Argm& argm, Error_list& exceptions) {
  if(argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  if (!argm.input.is_default())
    exceptions.add_error(Exception(Argm::False, "is_default_input"));
  return 0;}

// returns one if the output stream is not the default_stream
int b_is_default_output(const Argm& argm, Error_list& exceptions) {
  if(argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  if (!argm.output.is_default())
    exceptions.add_error(Exception(Argm::False, "is_default_output"));
  return 0;}

// returns one if the error stream is not the default_stream
int b_is_default_error(const Argm& argm, Error_list& exceptions) {
  if(argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  if (!argm.error.is_default())
    exceptions.add_error(Exception(Argm::False, "is_default_error"));
  return 0;}

// print the last exception that was thrown by this function
int b_last_exception(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->last_exception() <<"\n";
    argm.output.flush();
    return 0;}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_last_execution_time(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->last_execution_time();
    argm.output.flush();
    return 0;}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// print the environment that the shell started in
int b_list_environment(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos) {
      if (i != environ) argm.output <<" ";
      argm.output <<word_from_value(
        word_from_value(src.substr(0, div)) + " " +
        word_from_value(src.substr(div+1)));}}
  return 0;}

// prints the binaries that have been called, and all builtins and functions
int b_list_executables(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (auto i: executable_map)
    argm.output <<(i == *executable_map.begin()? "": " ") <<i.first;
  return 0;}

// prints all variables in the local variable map
int b_list_locals(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (auto i: *argm.parent_map())
    argm.output <<(i.first == argm.parent_map()->begin()->first? "": " ")
                <<i.first;
  argm.locals_listed();
  return 0;}

// add a variable to the variable map until the enclosing function terminates
int b_local(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.local(argm[1], argm[2]);
  return 0;}

// list the files specified by the arguments if they exist
int b_ls(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  struct stat sb;
  bool found = false;
  for (auto i: argm) if (!stat(i.c_str(), &sb)) {
    argm.output <<i <<"\n";
    found = true;}
  argm.output.flush();
  if (!found) {
    std::string tried(argm[1]);
    for (auto i: argm.subrange(2)) tried += " " + i;
    exceptions.add_error(Exception(Argm::File_not_found, tried)); }
  return 0;}

// ignore arguments, argfunctions, and then do nothing
int b_nop(const Argm& argm, Error_list& exceptions) {
  return Variable_map::dollar_question;}

// print the process id of the shell
int b_getpid(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<(unsigned) getpid();
  return 0;}

// print the parent process id of the shell
int b_getppid(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<(unsigned) getppid();
  return 0;}

// add the given exception as a replacement for the current one (if nothing
// afterwards fails)
int b_replace_exception(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!global_stack.in_exception_handler())
    throw Exception(Argm::Not_catching_exception);
  Argm new_exception(argm.subrange(1), argm.argfunction(),
                     Variable_map::global_map,
                     argm.input, argm.output.child_stream(), argm.error);
  exceptions.replace_error(new_exception);
  return 0;}

// return the value given by the argument
int b_return(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {return my_strtoi(argm[1]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// remove executable with name $1 from executable map
int b_rm_executable(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2)
    exceptions.add_error(Exception(Argm::Bad_argc, argm.argc()-1, 1, 0));
  if (argm.argfunction())
    exceptions.add_error(Exception(Argm::Excess_argfunction));
  if (global_stack.unwind_stack()) return 0;
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map());
  Base_executable *e = executable_map.find_second(lookup);
  if (dynamic_cast<Builtin*>(e))
    exceptions.add_error(Exception(Argm::Illegal_function_name, argm[1]));
  else if (!executable_map.erase(*(argm.begin()+1)))
    exceptions.add_error(Exception(Argm::Function_not_found, argm[1]));
  else; // successfully removed executable
  return 0;}

// run the argfunction having set local variables according to the given
// prototype
int b_scope(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm prototype_argm(argm.parent_map(), argm.input, argm.output, argm.error);
  tokenize_words(argm[argm.argc()-1], std::back_inserter(prototype_argm));
  Prototype prototype(prototype_argm.begin(), prototype_argm.end(), false);
  Argm invoking_argm(argm.subrange(0, 1), nullptr, argm.parent_map(),
                     argm.input, argm.output, argm.error);
  return (*argm.argfunction()).prototype_execute(invoking_argm, prototype,
                                                    exceptions);}

// modify variable $1 as a selection according to $2
int b_selection_set(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  std::list<Entry_pattern> focus;
  str_to_entry_pattern_list(argm.get_var(argm[1]), focus);
  std::string change = *(argm.begin()+2);
  for (auto i: argm.subrange(3)) change += ' ' + i;
  str_to_entry_pattern_list(change, focus);
  argm.set_var(argm[1], entry_pattern_list_to_str(focus.begin(), focus.end()));
  return 0;}

// set variable $1 to $*2
// returns 1 if the variable does not exist
int b_set(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  if (isargvar(argm[1])) throw Exception(Argm::Illegal_variable_name, argm[1]);
  std::string dest("");
  for (auto i: argm.subrange(2, 1)) dest += i + ' ';
  dest += argm.back();
  argm.set_var(argm[1], dest);
  return 0;}

// Set the configurable message for fallback_handler
// When this is given a prototype, just use the collected args directly
int b_set_fallback_message(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  fallback_message = "";
  for (auto i: argm.subrange(1, 1)) fallback_message += i + " ";
  fallback_message += argm.back();
  return 0;}

// Set the number of exceptions that can be thrown inside .collect_errors_*
// before they exit early
int b_set_max_collectible_exceptions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    global_stack.max_collect = my_strtoi(argm[1], 1,INT_MAX);
    return 0;}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// Set the number of exceptions that can be thrown by catch blocks after
// max_collectible_exceptions have already been thrown
int b_set_max_extra_exceptions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    global_stack.max_extra = my_strtoi(argm[1], 0, INT_MAX);
    return 0;}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// Set the maximum number of nesting levels where functions call functions
// before completing.
int b_set_max_nesting(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    global_stack.max_nesting = my_strtoi(argm[1], 0, INT_MAX);
    return 0;}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// run the first argument as if it was a script, passing additional arguments
// to that script
int b_source(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  struct stat sb;
  if (stat(argm[1].c_str(), &sb))
    throw Exception(Argm::File_open_failure, argm[1]);
  if (!(sb.st_mode & S_IXUSR)) throw Exception(Argm::Not_executable, argm[1]);
  Rwsh_istream_p src(new File_istream(argm[1]), false, false);
  Argm script_arg(argm.subrange(1), nullptr, argm.parent_map(),
                  argm.input, argm.output.child_stream(), argm.error);
  Command_stream command_stream(src, false);
  Arg_script script("", 0, exceptions);
  int ret = 0;
  while (!command_stream.fail() && !global_stack.unwind_stack())
    try {
      command_stream.getline(script, exceptions);
      if (command_stream.fail()) break;
      Argm command(script.interpret(script_arg, exceptions));
      if (global_stack.unwind_stack()) break;
      ret = executable_map.run(command, exceptions);}
    catch (Exception exception) {exceptions.add_error(exception);}
  return ret;}

// run the argument function once with each command in the specified function
// invocation
int b_stepwise(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map(),
                argm.input, argm.output.child_stream(), argm.error);
  Base_executable* e = executable_map.find_second(lookup);
  if (!e) throw Exception(Argm::Function_not_found, argm[1]);
  Function* f = dynamic_cast<Function*>(e);
  if (!f) return 0; //throw Exception(Argm::Not_a_function, argm[1]);
  // this must be caught and handled to use .stepwise recursively
  Variable_map locals(f->arg_to_param(lookup));
  Argm params(lookup.argv(), lookup.argfunction(), &locals,
              lookup.input, lookup.output, lookup.error);
  int ret = 0;
  for (auto j: f->body) {
    Argm body_i(j.interpret(params, exceptions));
    if (global_stack.unwind_stack()) break;
    Argm body(".mapped_argfunction", body_i.argv(), nullptr,
              body_i.parent_map(), body_i.input, body_i.output, body_i.error);
    ret  = (*argm.argfunction())(body, exceptions);
    (void) global_stack.remove_exceptions(".continue", exceptions);
    if (global_stack.remove_exceptions(".break", exceptions) ||
        global_stack.unwind_stack()) break;}
  f->unused_var_check(&locals, exceptions);
  return ret;} // last return value from argfunction

// run the argfunction and store its output in the variable $1
// returns the last return from the argfunction
int b_store_output(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  if (isargvar(argm[1])) throw Exception(Argm::Illegal_variable_name, argm[1]);
  Substitution_stream text;
  Argm mapped_argm(argm.parent_map(),
                   argm.input, text.child_stream(), argm.error);
  mapped_argm.push_back(".mapped_argfunction");
  int ret = (*argm.argfunction())(mapped_argm, exceptions);
  if (global_stack.unwind_stack() || ret) return ret;
  argm.set_var(argm[1], text.value());
  return 0;}

// return true if there is an executable in the executable map with key $1
int b_test_executable_exists(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  if (lookup[0] == ".argfunction") lookup[0] = ".mapped_argfunction";
  if (!executable_map.find_second(lookup))
    exceptions.add_error(Exception(Argm::False,
                                   "executable exists: " + argm[1]));
  return 0;}

// list the files specified by the arguments if they exist
int b_test_file_exists(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  struct stat sb;
  for (auto i: argm.subrange(1)) if (!stat(i.c_str(), &sb)) return 0;
  std::string err_val;
  for (auto i: argm.subrange(1)) err_val += " -e " + i;
  exceptions.add_error(Exception(Argm::False, err_val));
  return 0;}

// return true if two strings convert to a doubles and first is greater
int b_test_greater(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  double lhs, rhs;
  try {lhs = my_strtod(argm[1]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}
  try {rhs = my_strtod(argm[2]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[2]);}
  if (lhs <= rhs)
    exceptions.add_error(Exception(Argm::False, argm[1] + " -gt " + argm[2]));
  return 0;}

// return true if the first string is repeated in subsequent arguments
int b_test_in(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (auto j: argm.subrange(2)) if (argm[1] == j) return 0;
  auto err_val = argm[1] + " in";
  for (auto j: argm.subrange(2)) err_val += " " + j;
  exceptions.add_error(Exception(Argm::False, err_val));
  return 0;}

// return true if the string converts to a number
int b_test_is_number(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {(void) my_strtod(argm[1]);}
  catch (E_generic) {exceptions.add_error(Exception(Argm::False,
                        "is_number " + argm[1] + " - generic"));}
  catch (E_nan) {exceptions.add_error(Exception(Argm::False,
                        "is_number " + argm[1] + " - NaN"));}
  catch (E_range) {exceptions.add_error(Exception(Argm::False,
                        "is_number " + argm[1] + " - range"));}
  return 0;}

// return true if two strings convert to a doubles and first is less
int b_test_less(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  double lhs, rhs;
  try {lhs = my_strtod(argm[1]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}
  try {rhs = my_strtod(argm[2]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[2]);}
  if (lhs >= rhs)
    exceptions.add_error(Exception(Argm::False, argm[1] + " -lt " + argm[2]));
  return 0;}

// return true if the string is not empty
int b_test_not_empty(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else if (!argm[1].length())
    exceptions.add_error(Exception(Argm::False, " -n " + argm[1]));
  return 0;}

// returns true if the two strings
int b_test_number_equal(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  double lhs, rhs;
  try {lhs = my_strtod(argm[1]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}
  try {rhs = my_strtod(argm[2]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[2]);}
  if (lhs != rhs)
    exceptions.add_error(Exception(Argm::False, argm[1] + " -eq " + argm[2]));
  return 0;}

// return true if the two strings are the same
int b_test_string_equal(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else if (argm[1] != argm[2])
    exceptions.add_error(Exception(Argm::False, argm[1] + " == " + argm[2]));
  return 0;}

// return true if the two strings are different
int b_test_string_unequal(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else if (argm[1] == argm[2])
    exceptions.add_error(Exception(Argm::False, argm[1] + " != " + argm[2]));
  return 0;}

// throw the remaining arguments as an exception
int b_throw(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm new_exception(argm.subrange(1), argm.argfunction(),
                     Variable_map::global_map, default_input, default_output,
                     default_error);
  exceptions.add_error(new_exception);
  return 0;}

// enable readline if disabled, disable if enabled
int b_toggle_readline(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  readline_enabled = ! readline_enabled;
  return 0;}

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_total_execution_time(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    struct timeval val = focus->total_execution_time();
    argm.output <<val;
    argm.output.flush();
    return 0;}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// run the handler for specified exceptions
int b_try_catch_recursive(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm mapped_argm(argm.parent_map(),
                   argm.input, argm.output.child_stream(), argm.error);
  mapped_argm.push_back(".try_catch_recursive(body)");
  int ret = (*argm.argfunction())(mapped_argm, exceptions);
  if (global_stack.unwind_stack()) global_stack.catch_blocks(argm, exceptions);
  return ret;}

// print the type of executable with name $1 from executable map
int b_type(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2)
    exceptions.add_error(Exception(Argm::Bad_argc, argm.argc()-1, 1, 0));
  if (global_stack.unwind_stack()) return 0;
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  if (lookup[0] == ".argfunction") lookup[0] = ".mapped_argfunction";
  Base_executable *e = executable_map.find_second(lookup);
  if (!e) exceptions.add_error(Exception(Argm::Function_not_found, argm[1]));
  else if (dynamic_cast<Function*>(e)) argm.output <<"function\n";
  else if (dynamic_cast<Binary*>(e))   argm.output <<"file\n";
  else if (dynamic_cast<Builtin*>(e))  argm.output <<"builtin\n";
  else if (dynamic_cast<Command_block*>(e))  argm.output <<"argfunction\n";
  else std::abort(); // successfully removed executable
  return 0;}

// removes the given variable from the variable map. you could be really
// pedantic and throw an .undefined_variable if it doesn't exist, but the
// fact is that the requested state (one where this variable isn't set) is
// already the case, so it's hard to say what you're protecting people from.
int b_unset(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.unset_var(argm[1]);
  return 0;}

namespace {double sleep_requested = 0.0;}

// sleep for the specified number of microseconds
int b_usleep(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    int usec = my_strtoi(argm[1], 0, INT_MAX);
    sleep_requested += usec / 1000000.0;
    const long mega = (long)1E6;
    struct timespec delay = {usec / mega, (usec % mega) * 1000};
    if (!nanosleep(&delay, nullptr)) return 0;
    else if (errno == EINTR)                                       //not tested
      throw Exception(Argm::Interrupted_sleep);
    else throw Exception(Argm::Internal_error,                     //not tested
                         "nanosleep failed with code", errno);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// print the average number of microseconds longer that .usleep takes than it
// is requested to take.
int b_usleep_overhead(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  Argm usleep_argm(".usleep", Argm::Argv(), nullptr,
                   Variable_map::global_map, default_input, default_output,
                   default_error);
  Base_executable* focus = executable_map.find_second(Argm(usleep_argm));
  unsigned count = focus->execution_count();
  if (!count) throw Exception(Argm::Not_a_number, "");
  struct timeval slept = focus->total_execution_time();
  double total = slept.tv_sec + slept.tv_usec/1000000.0 - sleep_requested;
  argm.output <<(total/count);
  return 0;}

int b_var_add(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term;
  try {var_term = my_strtod(var_str);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, var_str);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, var_str);}
  catch (E_range) {throw Exception(Argm::Input_range, var_str);}
  double const_term;
  try {const_term = my_strtod(argm[2]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[2]);}
  double sum = var_term + const_term;
  if (sum == std::numeric_limits<double>::infinity() ||
      sum == -std::numeric_limits<double>::infinity())
    throw Exception(Argm::Result_range, var_str, argm[2]);
  std::ostringstream tmp;
  tmp <<sum;
  argm.set_var(argm[1], tmp.str());
  return 0;}

int b_var_subtract(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term;
  try {var_term = my_strtod(var_str);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, var_str);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, var_str);}
  catch (E_range) {throw Exception(Argm::Input_range, var_str);}
  double const_term;
  try {const_term = my_strtod(argm[2]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[2]);}
  double difference = var_term - const_term;
  if (difference >= DBL_MAX || difference <= -DBL_MAX)
    throw Exception(Argm::Result_range, var_str, argm[2]);
  std::ostringstream tmp;
  tmp <<difference;
  argm.set_var(argm[1], tmp.str());
  return 0;}

int b_var_divide(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term;
  try {var_term = my_strtod(var_str);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, var_str);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, var_str);}
  catch (E_range) {throw Exception(Argm::Input_range, var_str);}
  double const_term;
  try {const_term = my_strtod(argm[2]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[2]);}
  if (const_term == 0) throw Exception(Argm::Divide_by_zero, var_str);
  double quotient = var_term / const_term;
  if (quotient == 0 && var_term != 0)
    throw Exception(Argm::Result_range, var_str, argm[2]);
  std::ostringstream tmp;
  tmp <<quotient;
  argm.set_var(argm[1], tmp.str());
  return 0;}

int b_var_exists(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  bool any_exist = false;
  // need to go through the whole set to mark all as checked
  for (auto i: argm.subrange(1)) if (argm.var_exists(i)) any_exist = true;
  if (any_exist) return 0;
  std::string err_val ("var exists:");
  for (auto i: argm.subrange(1)) err_val += " " + i;
  exceptions.add_error(Exception(Argm::False, err_val));
  return 0;}

static const std::string version_str("0.3+");

// write to standard output the version of rwsh
int b_version(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<version_str;
  return 0;}

// return true if the given version string is compatible with the version
// of this shell
int b_version_compatible(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else if (argm[1] == version_str) return 0;
  else throw Exception(Argm::Version_incompatible, argm[1]);}

// prints the total amount of time the shell has not been waiting for user input
int b_waiting_for_binary(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<rwsh_clock.waiting_for_binary();
  argm.output.flush();
  return 0;}

// prints the total amount of time that has passed and the shell has not been
// waiting for other processes or the user
int b_waiting_for_shell(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<rwsh_clock.waiting_for_shell();
  argm.output.flush();
  return 0;}

// prints the total amount of time the shell has been waiting for user input
int b_waiting_for_user(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<rwsh_clock.waiting_for_user();
  argm.output.flush();
  return 0;}

// print the string corresponding to the executable in the executable map with
// key $1
int b_whence_function(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  if (lookup[0] == ".argfunction") lookup[0] = ".mapped_argfunction";
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->str() <<"\n";
    argm.output.flush();}
  else exceptions.add_error(Exception(Argm::Function_not_found, argm[1]));
  return 0;}

// if the filename has a leading dot, then check in current directory
// otherwise find the binary in $2 with filename $1
int b_which_path(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  std::vector<std::string> path;
  tokenize(argm[2], std::back_inserter(path),
           std::bind2nd(std::equal_to<char>(), ':'));
  for (auto j: path) {
    std::string test;
    if (argm[1].substr(0,1) == "/" || argm[1].substr(0,2) == "./" ||
        argm[1].substr(0,3) == "../")
      if (argm[1].substr(0, j.length()) != j) continue;
      else test = argm[1];
    else if (j.back() == '/') test = j + argm[1];
    else test = j + '/' + argm[1];
    struct stat sb;
    if (!stat(test.c_str(), &sb)) {
      argm.output <<test;
      return 0;}}
  exceptions.add_error(Exception(Argm::Binary_not_found, argm[1], argm[2]));
  return 0;}                                      // executable does not exist

// for each time that the arguments return true, run the argfunction
// returns the last return from the argfunction
int b_while(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  int ret = 0;
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map(),
              argm.input, argm.output.child_stream(), argm.error);
  while (executable_map.run_condition(lookup, exceptions)) {
    Argm mapped_argm(argm.parent_map(), argm.input, argm.output.child_stream(),
                     argm.error);
    mapped_argm.push_back(".mapped_argfunction");
    ret = (*argm.argfunction())(mapped_argm, exceptions);
    (void) global_stack.remove_exceptions(".continue", exceptions);
    if (global_stack.remove_exceptions(".break", exceptions) ||
        global_stack.unwind_stack()) return 0;}
  return ret;}
