// The functions that implement each of the builtin executables
//
// Copyright (C) 2006-2018 Samuel Newbold

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
#include "clock.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "file_stream.h"
#include "pipe_stream.h"
#include "prototype.h"
#include "read_dir.cc"
#include "rwshlib.h"
#include "selection.h"
#include "substitution_stream.h"
#include "tokenize.cc"

#include "function.h"
#include "argm_star_var.cc"

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
// returns the error returned from chdir
int b_cd(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  errno = 0;
  int ret = chdir(argm[1].c_str());
  if (!ret) return 0;
  else if (errno == ENOENT) ret = 1;
  else if (errno == ENOTDIR) ret = 2;
  else ret = 3;
  errno = 0;
  return ret;}

// run the argument function, collecting exceptions to be thrown as a group
// at the end, but terminating immediately if one of the specified exceptions
// are thrown
int b_collect_errors_except(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm blank(argm.parent_map(), argm.input, argm.output.child_stream(),
             argm.error);
  blank.push_back("rwsh.mapped_argfunction");
  std::vector<std::string> exceptional(argm.begin()+1, argm.end());
  return argm.argfunction()->collect_errors_core(blank, exceptional, true,
                                                 exceptions);}

// run the argument function, collecting exceptions to be thrown as a group
// at the end, but only until an exception is not one of the specified
// exceptions
int b_collect_errors_only(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm blank(argm.parent_map(), argm.input, argm.output.child_stream(),
             argm.error);
  blank.push_back("rwsh.mapped_argfunction");
  std::vector<std::string> exceptional(argm.begin()+1, argm.end());
  return argm.argfunction()->collect_errors_core(blank, exceptional, false,
                                                 exceptions);}

// echo arguments to standard output without space separation
int b_combine(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (Argm::const_iterator i = argm.begin()+1; i != argm.end(); ++i)
    argm.output <<*i;
  argm.output.flush();
  return 0;}

// echo arguments to standard output separated by space
int b_echo(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (Argm::const_iterator i = argm.begin()+1; i != argm.end()-1; ++i)
    argm.output <<*i <<" ";
  argm.output <<argm.back();
  argm.output.flush();
  return 0;}

#include <iostream>
// replace the shell with the given binary
int b_exec(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  int input = argm.input.fd(),
      output = argm.output.fd(),
      error = argm.error.fd();
  if (dup2(input, 0) < 0) std::cerr <<"dup2 didn't like changing input\n";
  if (dup2(output, 1) < 0) std::cerr <<"dup2 didn't like changing output\n";
  if (dup2(error, 2) < 0) std::cerr <<"dup2 didn't like changing error\n";
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
                default_input, default_output, default_error);
  Old_argv old_argv(lookup);
  char **env = argm.export_env();
  int ret = execve(lookup[0].c_str(), old_argv.argv(), env);
  Exception error_argm(Argm::Binary_not_found, argm[0]);
  executable_map.run(error_argm, exceptions);
  return ret;}

// exit the shell
int b_exit(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  Variable_map::exit_requested = true;
  return 0;}

/* Exception handler for exceptions that trigger exceptions in their exception
   handler, without possibly itself triggering exceptions. This prints its
   arguments prefixed by a configurable message.*/
int b_fallback_handler(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  argm.output <<fallback_message;
  for (Argm::const_iterator i = argm.begin()+1; i != argm.end()-1; ++i)
    argm.output <<*i <<" ";
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
  int ret = -1;
  Argm body(argm.parent_map(), argm.input, argm.output, argm.error);
  body.push_back("rwsh.mapped_argfunction");
  body.push_back("");
  for (Argm::const_iterator i = ++argm.begin(); i != argm.end(); ++i) {
    if (argm.argfunction()) {
      body[1] = *i;
      ret  = (*argm.argfunction())(body, exceptions);
      if (Named_executable::unwind_stack()) return -1;}
    else ret = 0;}
  return ret;}

// run the argfunction for line of input, passing that line as the argm
// returns the value returned by the argfunction
int b_for_each_line(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  int ret = -1;
  while(!argm.input.fail()) {
    std::string line;
    // shouldn't interfere with input being consumed by this builtin
    Argm body(argm.parent_map(), default_input, argm.output, argm.error);
    body.push_back("rwsh.mapped_argfunction");
    argm.input.getline(line);
    if (argm.input.fail() && !line.size()) break;
    tokenize(line, std::back_inserter(body),
             std::bind2nd(std::equal_to<char>(), ' '));
    ret = (*argm.argfunction())(body, exceptions);}
  return ret;}

#include "plumber.h"
int b_fork(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  // argfunction optional
  int ret = 0;
  if (!fork()) {
    plumber.after_fork();
    Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(),
                argm.parent_map(),
                argm.input, argm.output.child_stream(), argm.error);
    ret = executable_map.run(lookup, exceptions);
    executable_map.unused_var_check_at_exit();
    std::exit(ret);}
  else plumber.wait(&ret);
  return ret;}

// add argfunction to executable map with name $1
int b_function(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  else if (is_binary_name(argm[1])) return 1;
  Argm lookup(argm.begin()+1, argm.end(), NULL, argm.parent_map(),
                default_input, default_output, default_error);
  Base_executable *e = executable_map.find(lookup);
  if (e && dynamic_cast<Builtin*>(e)) return 2;
  else if (is_argfunction_name(argm[1])) return 3;
  else if (!argm.argfunction()) {
    return 4 * !executable_map.erase(*(argm.begin()+1));}
  else {
    executable_map.set(new Function(argm[1], argm.end(), argm.end(), true,
                                    *argm.argfunction()));
    return 0;}}

// add argfunction to executable map with name $1 and arguments $*2
// the arguments must include all flags that can be passed to this function
int b_function_all_flags(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  else if (is_binary_name(argm[1])) return 1;
  else if (is_argfunction_name(argm[1])) return 3;
  Argm lookup(argm.begin()+1, argm.begin()+2, NULL, argm.parent_map(),
                default_input, default_output, default_error);
  Base_executable *e = executable_map.find(lookup);
  if (dynamic_cast<Builtin*>(e)) return 2;
  else if (!argm.argfunction()) {
    return 4 * !executable_map.erase(*(argm.begin()+1));}
  else {
    Function *focus = new Function(argm[1], argm.begin()+2, argm.end(), false,
                                   *argm.argfunction());
    executable_map.set(focus);
    return 0;}}

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
  argm.output <<Base_executable::max_collect;
  return 0;}

// Get the number of exceptions that can be thrown by catch blocks after
// max_collectible_exceptions have already been thrown
int b_get_max_extra_exceptions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<Base_executable::max_extra;
  return 0;}

// Get the maximum number of nesting levels where functions call functions
// before completing.
int b_get_max_nesting(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<Base_executable::max_nesting;
  return 0;}

// add a variable to the variable map that will remain after the enclosing
// function terminates
int b_global(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else return argm.global(argm[1], argm[2]);}

namespace {
bool in_if_block = false, successful_condition = false,
  conditional_block_exception = false;

int if_core(const Argm& argm, Error_list& exceptions, bool logic, bool els) {
  if (!in_if_block) throw Exception(Argm::Else_without_if);
  else if (successful_condition) return Variable_map::dollar_question;
  else {
    Argm lookup(argm.begin()+1, argm.end(), 0, argm.parent_map(),
                  argm.input, argm.output.child_stream(), argm.error);
    if (els || logic == !executable_map.run(lookup, exceptions)) {
      if (Named_executable::unwind_stack()) return -1;
      in_if_block = false;
      int ret;
      if (argm.argfunction()) {
        Argm mapped_argm(argm.parent_map(),
                         argm.input, argm.output.child_stream(), argm.error);
        mapped_argm.push_back("rwsh.mapped_argfunction");
        ret = (*argm.argfunction())(mapped_argm, exceptions);}
      else ret = 0;
      if (Base_executable::unwind_stack()) conditional_block_exception = true;
      else if (in_if_block) {
        in_if_block = false;
        throw Exception(Argm::Bad_if_nest);}
      else in_if_block = successful_condition = true;
      return ret;}
    else return 0;}}
}

// run argfunction if $* returns true
// returns the value returned by the argfunction
int b_if(const Argm& argm, Error_list& exceptions) {
  try {
    if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (conditional_block_exception) conditional_block_exception = false;
    else if (in_if_block) throw Exception(Argm::If_before_else);
    in_if_block = true, successful_condition = false;
    return if_core(argm, exceptions, true, false);}
  catch (Exception exception) {
    in_if_block = true,
      successful_condition = false;
    conditional_block_exception = true;
    throw exception;}}

// run argfunction if successful_condition is false and $* returns true
// returns the value returned by the argfunction
int b_else_if(const Argm& argm, Error_list& exceptions) {
  try {
    if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (conditional_block_exception) return 0;
    else return if_core(argm, exceptions, true, false);}
  catch (Exception exception) {
    conditional_block_exception = true;
    throw exception;}}

// run argfunction if successful_condition is false and $* returns false
// returns the value returned by the argfunction
int b_else_if_not(const Argm& argm, Error_list& exceptions) {
  try {
    if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (conditional_block_exception) return 0;
    else return if_core(argm, exceptions, false, false);}
  catch (Exception exception) {
    conditional_block_exception = true;
    throw exception;}}

// run argfunction if successful_condition is false
// returns the value returned by the argfunction
int b_else(const Argm& argm, Error_list& exceptions) {
  try {
    int ret = 0;
    if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
    else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
    else if (conditional_block_exception) conditional_block_exception = false;
    else ret = if_core(argm, exceptions, false, true);
    in_if_block = false;
    return ret;}
  catch (Exception exception) {
    in_if_block = false;
    throw exception;}}

// prints a list of all internal functions
int b_internal_functions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  argm.output <<Argm::exception_names[1];
  for (int i = 2; i < Argm::Exception_count; ++i)
    argm.output <<"\n" <<Argm::exception_names[i];
  return 0;}

// returns one if the input stream is not the default_stream
int b_is_default_input(const Argm& argm, Error_list& exceptions) {
  if(argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  return !argm.input.is_default();}

// returns one if the output stream is not the default_stream
int b_is_default_output(const Argm& argm, Error_list& exceptions) {
  if(argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  return !argm.output.is_default();}

// returns one if the error stream is not the default_stream
int b_is_default_error(const Argm& argm, Error_list& exceptions) {
  if(argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  return !argm.error.is_default();}

// print the environment that the shell started in
int b_list_environment(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    if (src.find_first_of("()") != std::string::npos) continue;
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
  for (Executable_map::const_iterator i=executable_map.begin();
       i != executable_map.end(); ++i)
    argm.output <<(i == executable_map.begin()? "": " ") <<i->first;
  return 0;}

// prints all variables in the local variable map
int b_list_locals(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  for (Variable_map::iterator i=argm.local_begin(); i != argm.local_end(); ++i)
    argm.output <<(i == argm.local_begin()? "": " ") <<i->first;
  argm.locals_listed();
  return 0;}

// add a variable to the variable map until the enclosing function terminates
int b_local(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else return argm.local(argm[1], argm[2]);}

// list the files specified by the arguments if they exist
int b_ls(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  struct stat sb;
  int ret = 1;
  for (Argm::const_iterator i=argm.begin(); i != argm.end(); ++i)
    if (!stat(i->c_str(), &sb)) {
      argm.output <<*i <<"\n";
      ret = 0;}
  argm.output.flush();
  return ret;}

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

// return the value given by the argument
int b_return(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {return my_strtoi(argm[1]);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// run the argfunction having set local variables according to the given
// prototype
int b_scope(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  else if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm prototype_argm(argm.parent_map(),
                      default_input, default_output, default_error);
  tokenize_words(argm[argm.argc()-1], std::back_inserter(prototype_argm));
  Prototype prototype(prototype_argm.begin(), prototype_argm.end(), false);
  Argm invoking_argm(argm.begin(), argm.end()-1, NULL, argm.parent_map(),
                     default_input, default_output, default_error);
  int ret = (*argm.argfunction()).prototype_execute(invoking_argm, prototype,
                                                    exceptions);
  if (Named_executable::unwind_stack()) return -1;
  else return ret;}

// modify variable $1 as a selection according to $2
int b_selection_set(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  std::list<Entry_pattern> focus;
  str_to_entry_pattern_list(argm.get_var(argm[1]), focus);
  std::string change = *(argm.begin()+2);
  for (Argm::const_iterator i = argm.begin()+3; i != argm.end(); ++i)
    change += ' ' + *i;
  str_to_entry_pattern_list(change, focus);
  argm.set_var(argm[1], entry_pattern_list_to_str(focus.begin(), focus.end()));
  return 0;}

// set variable $1 to $*2
// returns 1 if the variable does not exist
int b_set(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  if (isargvar(argm[1])) return 2;
  std::string dest("");
  for (Argm::const_iterator i = argm.begin()+2; i != argm.end()-1; ++i)
    dest += *i + ' ';
  dest += argm.back();
  return argm.set_var(argm[1], dest);}

// Set the configurable message for fallback_handler
// When this is given a prototype, just use the collected args directly
int b_set_fallback_message(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  fallback_message = "";
  for (Argm::const_iterator i = argm.begin()+1; i != argm.end()-1; ++i)
    fallback_message += *i + " ";
  fallback_message += argm.back();
  return 0;}

// Set the number of exceptions that can be thrown inside .collect_errors_*
// before they exit early
int b_set_max_collectible_exceptions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    Base_executable::max_collect = my_strtoi(argm[1], 1,INT_MAX);
    return 0;}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// Set the number of exceptions that can be thrown by catch blocks after
// max_collectible_exceptions have already been thrown
int b_set_max_extra_exceptions(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    Base_executable::max_extra = my_strtoi(argm[1], 0, INT_MAX);
    return 0;}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// Set the maximum number of nesting levels where functions call functions
// before completing.
int b_set_max_nesting(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    Base_executable::max_nesting = my_strtoi(argm[1], 0, INT_MAX);
    return 0;}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// run the first argument as if it was a script, passing additional arguments
// to that script
// returns last return value from script, -1 if empty
int b_source(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  struct stat sb;
  if (stat(argm[1].c_str(), &sb))
    throw Exception(Argm::File_open_failure, argm[1]);
  if (!(sb.st_mode & S_IXUSR)) throw Exception(Argm::Not_executable, argm[1]);
  Rwsh_istream_p src(new File_istream(argm[1]), false, false);
  Argm script_arg(argm.begin()+1, argm.end(), 0, argm.parent_map(),
                  argm.input, argm.output.child_stream(), argm.error);
  Command_stream command_stream(src, false);
  Arg_script script("", 0);
  int ret = -1;
  while (!command_stream.fail() && !Named_executable::unwind_stack()) {
    try {
      command_stream >> script;
      if (command_stream.fail()) break;
      Argm command(script.interpret(script_arg, exceptions));
      ret = executable_map.run(command, exceptions);}
    catch (Exception exception) {
      ret = executable_map.run(exception, exceptions);}}
  return ret;}

// run the argument function once with each command in the specified function
// invocation
int b_stepwise(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm lookup(argm.begin()+1, argm.end(), 0, argm.parent_map(),
                argm.input, argm.output.child_stream(), argm.error);
  Base_executable* e = executable_map.find(lookup);
  if (!e) return 1;  // executable not found
  Function* f = dynamic_cast<Function*>(e);
  if (!f) return 2; // the named executable is not a function
  int ret = -1;
  for (Command_block::const_iterator i = f->body.begin();
       i != f->body.end(); ++i) {
    Argm body_i(i->interpret(lookup, exceptions));
    Argm body("rwsh.mapped_argfunction", body_i.begin(), body_i.end(), 0,
              argm.parent_map(), body_i.input, body_i.output, body_i.error);
    ret  = (*argm.argfunction())(body, exceptions);
    if (Named_executable::unwind_stack()) {
      ret = -1;
      break;}}
  return ret;} // last return value from argfunction

// run the argfunction and store its output in the variable $1
// returns the last return from the argfunction
int b_store_output(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  if (isargvar(argm[1])) return 2;
  Substitution_stream text;
  Argm mapped_argm(argm.parent_map(),
                   argm.input, text.child_stream(), argm.error);
  mapped_argm.push_back("rwsh.mapped_argfunction");
  int ret = (*argm.argfunction())(mapped_argm, exceptions);
  if (Named_executable::unwind_stack()) return -1;
  if (ret) return ret;
  argm.set_var(argm[1], text.value());
  return 0;}

// return true if the two strings are the same
int b_test_string_equal(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else return argm[1] != argm[2];} // C++ and shell have inverted logic

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
  return lhs <= rhs;} // C++ and shell have inverted logic

// return true if the string converts to a number
int b_test_is_number(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    (void) my_strtod(argm[1]);
    return 0;}
  catch (E_generic) {return 1;}
  catch (E_nan) {return 1;}
  catch (E_range) {return 2;}}

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
  return lhs >= rhs;} // C++ and shell have inverted logic

// return true if the string is not empty
int b_test_not_empty(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else return !argm[1].length();} // C++ and shell have inverted logic

// return true if the two strings are different
int b_test_string_unequal(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else return argm[1] == argm[2];} // C++ and shell have inverted logic

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
  return lhs != rhs;} // C++ and shell have inverted logic

// throw the remaining arguments as an exception
int b_throw(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm new_exception(argm.begin()+1, argm.end(), argm.argfunction(),
                     Variable_map::global_map,
                     argm.input, argm.output.child_stream(), argm.error);
  exceptions.add_error(new_exception);
  return -1;}

// enable readline if disabled, disable if enabled
int b_toggle_readline(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  readline_enabled = ! readline_enabled;
  return 0;}

// run the handler for specified exceptions
int b_try_catch_recursive(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  Argm mapped_argm(argm.parent_map(),
                   argm.input, argm.output.child_stream(), argm.error);
  mapped_argm.push_back("rwsh.mapped_argfunction");
  int ret = (*argm.argfunction())(mapped_argm, exceptions);
  if (Named_executable::unwind_stack()) {
    Base_executable::catch_blocks(argm, exceptions);
    return -1;}
  return ret;}

// removes the given variable from the variable map. you could be really
// pedantic and throw an rwsh.undefined_variable if it doesn't exist, but the
// fact is that the requested state (one where this variable isn't set) is
// already the case, so it's hard to say what you're protecting people from.
int b_unset(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  else return argm.unset_var(argm[1]);}

namespace {double sleep_requested = 0.0;}

// sleep for the specified number of microseconds
int b_usleep(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  try {
    int delay = my_strtoi(argm[1], 0, INT_MAX);
    sleep_requested += delay / 1000000.0;
    return usleep(delay);}
  catch (E_generic) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Exception(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Exception(Argm::Input_range, argm[1]);}}

// print the average number of microseconds longer that .usleep takes than it
// is requested to take.
int b_usleep_overhead(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 1) throw Exception(Argm::Bad_argc, argm.argc()-1, 0, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  Argm usleep_argm(".usleep", argm.end(), argm.end(), NULL,
                   Variable_map::global_map, default_input, default_output,
                   default_error);
  Base_executable* focus = executable_map.find(Argm(usleep_argm));
  unsigned count = focus->execution_count();
  if (!count) return 1;
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
  for (Argm::const_iterator i=argm.begin()+1; i != argm.end(); ++i)
    if (argm.var_exists(*i)) any_exist = true;
  return !any_exist;}

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
int b_which_executable(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Base_executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->str();
    argm.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_which_execution_count(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
                default_input, default_output, default_error);
  Base_executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->execution_count();
    argm.output.flush();
    return 0;}
  else return 1;}          // executable does not exist

// print the last exception that was thrown by this function
int b_which_last_exception(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
                default_input, default_output, default_error);
  Base_executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->last_exception();
    argm.output.flush();
    return 0;}
  else return 1;}          // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_which_last_execution_time(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
                default_input, default_output, default_error);
  Base_executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->last_execution_time();
    argm.output.flush();
    return 0;}
  else return 1;}          // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_which_total_execution_time(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
                default_input, default_output, default_error);
  Base_executable* focus = executable_map.find(lookup);
  if (focus) {
    struct timeval val = focus->total_execution_time();
    argm.output <<val;
    argm.output.flush();
    return 0;}
  else return 1;}          // executable does not exist

// find the binary in $2 with filename $1
int b_which_path(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 3) throw Exception(Argm::Bad_argc, argm.argc()-1, 2, 0);
  if (argm.argfunction()) throw Exception(Argm::Excess_argfunction);
  std::vector<std::string> path;
  tokenize_strict(argm[2], std::back_inserter(path),
                  std::bind2nd(std::equal_to<char>(), ':'));
  for (std::vector<std::string>::iterator i = path.begin(); i != path.end();
       ++i) {
    std::string test = *i + '/' + argm[1];
    struct stat sb;
    if (!stat(test.c_str(), &sb)) {
      argm.output <<test;
      return 0;}}
  return 1;} // executable does not exist

// prints the last return value of the executable with named $1
int b_which_return(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
                default_input, default_output, default_error);
  Base_executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->last_ret();
    argm.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// return true if there is an executable in the executable map with key $1
int b_which_test(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() != 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), argm.parent_map(),
              default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  return !executable_map.find(lookup);}

// for each time that the arguments return true, run the argfunction
// returns the last return from the argfunction
int b_while(const Argm& argm, Error_list& exceptions) {
  if (argm.argc() < 2) throw Exception(Argm::Bad_argc, argm.argc()-1, 1, 0);
  if (!argm.argfunction()) throw Exception(Argm::Missing_argfunction);
  int ret = -1;
  Argm lookup(argm.begin()+1, argm.end(), 0, argm.parent_map(),
              argm.input, argm.output.child_stream(), argm.error);
  while (!executable_map.run(lookup, exceptions)) {
    if (Named_executable::unwind_stack()) return -1;
    Argm mapped_argm(argm.parent_map(), argm.input, argm.output.child_stream(),
                     argm.error);
    mapped_argm.push_back("rwsh.mapped_argfunction");
    ret = (*argm.argfunction())(mapped_argm, exceptions);
    if (Named_executable::unwind_stack()) return -1;}
  return ret;}

