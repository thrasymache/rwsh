// The functions that implement each of the builtin executables
//
// Copyright (C) 2006-2019 Samuel Newbold

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
const double dummy_double = -4.2e+42;
const double dummy_int = -424242424;
std::string fallback_message = "Exception for failed handler. "
                               "Original exception with call stack:\n";

std::string my_dtostr(double in) {
  std::ostringstream ss;
  ss <<in;
  return ss.str();}
} // end unnamed namespace

double my_strtod(const std::string& val,  Error_list& errors) {
  try {return my_strtod(val);}
  catch (E_generic) {errors.add_error(Exception(Argm::Not_a_number, val));}
  catch (E_nan) {errors.add_error(Exception(Argm::Not_a_number, val));}
  catch (E_range) {errors.add_error(Exception(Argm::Input_range, val));}
  return dummy_double;} // any use of this value is a failure to stop on error

int my_strtoi(const std::string& val, int min, int max, Error_list& errors) {
  try {return my_strtoi(val, min, max);}
  catch (E_generic) {errors.add_error(Exception(Argm::Not_a_number, val));}
  catch (E_nan) {errors.add_error(Exception(Argm::Not_a_number, val));}
  catch (E_range) {errors.add_error(Exception(Argm::Input_range, val));}
  return dummy_int;} // any use of this value is a failure to stop on error

// print the number of arguments passed
void b_argc(const Argm& argm, Error_list& exceptions) {
  argm.output <<argm.argc()-1;}

// add binary to executable map with name $1
void b_binary(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map());
  struct stat sb;
  if (stat(lookup[0].c_str(), &sb))
    exceptions.add_error(Exception(Argm::Binary_does_not_exist, lookup[0]));
  else if (executable_map.find_second(lookup))
    exceptions.add_error(Exception(Argm::Executable_already_exists, lookup[0]));
  else executable_map.set(new Binary(lookup[0]));}

// change the current directory to the one given
void b_cd(const Argm& argm, Error_list& exceptions) {
  errno = 0;
  if (!chdir(argm[1].c_str()));
  else if (errno == ENOENT)
    exceptions.add_error(Exception(Argm::Directory_not_found, argm[1]));
  else if (errno == ENOTDIR)
    exceptions.add_error(Exception(Argm::Not_a_directory, argm[1]));
  // not tested. time for additional errors to be differentiated
  else exceptions.add_error(Exception(Argm::Internal_error, errno));
  errno = 0;}

// run the argument function, collecting exceptions to be thrown as a group
// at the end, but terminating immediately if one of the specified exceptions
// are thrown
void b_collect_errors_except(const Argm& argm, Error_list& exceptions) {
  global_stack.collect_errors_core(argm, true, exceptions);}

// run the argument function, collecting exceptions to be thrown as a group
// at the end, but only until an exception is not one of the specified
// exceptions
void b_collect_errors_only(const Argm& argm, Error_list& exceptions) {
  global_stack.collect_errors_core(argm, false, exceptions);}

// echo arguments to standard output without space separation
void b_combine(const Argm& argm, Error_list& exceptions) {
  for (auto i: argm.subrange(1)) argm.output <<i;
  argm.output.flush();}

// disable readline regardless of status
void b_disable_readline(const Argm& argm, Error_list& exceptions) {
  readline_enabled = false;}

// echo arguments to standard output separated by space
void b_echo(const Argm& argm, Error_list& exceptions) {
  for (auto i: argm.subrange(1, 1)) argm.output <<i <<" ";
  argm.output <<argm.back();
  argm.output.flush();}

// enable readline regardless of status
void b_enable_readline(const Argm& argm, Error_list& exceptions) {
  readline_enabled = true;}

// echo arguments to standard error separated by space
void b_error(const Argm& argm, Error_list& exceptions) {
  for (auto i: argm.subrange(1, 1)) argm.error <<i <<" ";
  argm.error <<argm.back();
  argm.error.flush();}

// replace the shell with the given binary
void b_exec(const Argm& argm, Error_list& exceptions) {
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
  if (errno == ENOENT)   // depends on the error handlers to indicate failure
    exceptions.add_error(Exception(Argm::Binary_does_not_exist, argm[1]));
  else if (errno == EACCES || errno == ENOEXEC)
    exceptions.add_error(Exception(Argm::Not_executable, argm[1]));
  else exceptions.add_error(Exception(Argm::Exec_failed, argm[1], errno));}

// print the number of times that the executable in the executable map with
// key $1 has been run
void b_execution_count(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->execution_count();
    argm.output.flush();}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// exit the shell with the specified exit value
void b_exit(const Argm& argm, Error_list& exceptions) {
  int val = my_strtoi(argm[1], -255, 255, exceptions);
  if (global_stack.unwind_stack()) return; // another chance to give status
  global_stack.request_exit(val);
  if (gc_state.in_if_block) gc_state.exception_thrown = true;}

/* Exception handler for exceptions that trigger exceptions in their exception
   handler, without possibly itself triggering exceptions. This prints its
   arguments prefixed by a configurable message.*/
void b_fallback_handler(const Argm& argm, Error_list& exceptions) {
  argm.output <<fallback_message;
  for (auto i: argm.subrange(1, 1)) argm.output <<i <<" ";
  argm.output <<argm.back() <<"\n";
  argm.output.flush();}
#if 0
    argm.error <<*i <<" ";
  argm.error <<argm.back() <<"\n";
  argm.error.flush();}
#endif

// run the argfunction for each argument, passing that value as the argument
void b_for(const Argm& argm, Error_list& exceptions) {
  Argm body(argm.parent_map(), argm.input, argm.output, argm.error);
  body.push_back(".mapped_argfunction");
  body.push_back("");
  for (auto i: argm.subrange(1))
    if (argm.argfunction()) {
      body[1] = i;
      (*argm.argfunction())(body, exceptions);
      (void) global_stack.remove_exceptions(".continue", exceptions);
      if (global_stack.remove_exceptions(".break", exceptions) ||
          global_stack.unwind_stack()) return;}}

// run the argfunction for line of input, passing that line as the argm
void b_for_each_line(const Argm& argm, Error_list& exceptions) {
  while(!argm.input.fail()) {
    std::string line;
    // shouldn't interfere with input being consumed by this builtin
    Argm body(argm.parent_map(), default_input, argm.output, argm.error);
    body.push_back(".mapped_argfunction");
    argm.input.getline(line);
    if (argm.input.fail() && !line.size()) break;
    tokenize(line, std::back_inserter(body),
             std::bind2nd(std::equal_to<char>(), ' '));
    (*argm.argfunction())(body, exceptions);
    (void) global_stack.remove_exceptions(".continue", exceptions);
    if (global_stack.remove_exceptions(".break", exceptions) ||
        global_stack.unwind_stack()) return;}}

void b_fork(const Argm& argm, Error_list& exceptions) {
  int status = 0;
  if (!fork()) {
    plumber.after_fork();
    Argm lookup(argm.subrange(1), argm.argfunction(),
                argm.parent_map(),
                argm.input, argm.output.child_stream(), argm.error);
    executable_map.run(lookup, exceptions);
    if (global_stack.unwind_stack()) {
      global_stack.exception_handler(exceptions);
      if (!global_stack.exit_value()) global_stack.request_exit(-1);}
    executable_map.unused_var_check_at_exit();
    std::exit(global_stack.exit_value());}
  else plumber.wait(&status);
  if (WIFEXITED(status) && WEXITSTATUS(status))
    exceptions.add_error(Exception(Argm::Return_code, WEXITSTATUS(status)));}

// add argfunction to executable map with name $1
void b_function(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map());
  Base_executable *e = executable_map.find_second(lookup);
  if (is_argfunction_name(argm[1]) || dynamic_cast<Builtin*>(e))
    exceptions.add_error(Exception(Argm::Illegal_function_name, argm[1]));
  else {
      argm.error <<"deprecated non-prototype: " <<argm.str() <<"\n";
      executable_map.set(new Function(argm[1], true, *argm.argfunction()));
  }}

// add argfunction to executable map with name $1 and arguments $*2
// the arguments must include all flags that can be passed to this function
void b_function_all_flags(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1, argm.argc()-2), nullptr, argm.parent_map());
  Base_executable *e = executable_map.find_second(lookup);
  if (is_argfunction_name(argm[1]) || dynamic_cast<Builtin*>(e))
    exceptions.add_error(Exception(Argm::Illegal_function_name, argm[1]));
  else executable_map.set(new Function(argm[1], argm.subrange(2),
                          *argm.argfunction()));}

// Get the configurable message for fallback_handler
void b_get_fallback_message(const Argm& argm, Error_list& exceptions) {
  argm.output <<fallback_message;}

// Get the number of exceptions that can be thrown inside .collect_errors_*
// before they exit early
void b_get_max_collectible_exceptions(const Argm& argm,Error_list& exceptions) {
  argm.output <<global_stack.max_collect;}

// Get the number of exceptions that can be thrown by catch blocks after
// max_collectible_exceptions have already been thrown
void b_get_max_extra_exceptions(const Argm& argm, Error_list& exceptions) {
  argm.output <<global_stack.max_extra;}

// Get the maximum number of nesting levels where functions call functions
// before completing.
void b_get_max_nesting(const Argm& argm, Error_list& exceptions) {
  argm.output <<global_stack.max_nesting;}

// add a variable to the variable map that will remain after the enclosing
// function terminates
void b_global(const Argm& argm, Error_list& exceptions) {
  argm.global(argm[1], argm[2]);}

namespace {
void if_core(const Argm& argm, Error_list& exceptions,
            Conditional_state& state, bool logic, bool is_else) {
  if (!state.in_if_block) throw Exception(Argm::Else_without_if);
  else if (!state.successful_condition) {
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
        (*argm.argfunction())(mapped_argm, exceptions);}
      if (global_stack.unwind_stack())
        state.exception_thrown = ! is_else;
      else if (state.in_if_block) {
        state.in_if_block = false;
        throw Exception(Argm::Bad_if_nest);}
      else state.in_if_block = state.successful_condition = true;}
    state.in_if_block = true;}}
}

// run argfunction if $* doesn't throw an exception
void b_if(const Argm& argm, Error_list& exceptions) {
  try {
    if (gc_state.exception_thrown)
      gc_state.successful_condition = gc_state.exception_thrown = false;
    else if (gc_state.in_if_block) throw Exception(Argm::If_before_else);
    gc_state.in_if_block = true;
    if_core(argm, exceptions, gc_state, true, false);}
  catch (Exception exception) {
    gc_state.exception_thrown = true;
    throw exception;}}

// run argfunction if successful_condition is false and $* doesn't throw
void b_else_if(const Argm& argm, Error_list& exceptions) {
  try {if (!gc_state.exception_thrown)
    if_core(argm, exceptions, gc_state, true, false);}
  catch (Exception exception) {
    gc_state.exception_thrown = true;
    throw exception;}}

// run argfunction if successful_condition is false and $* doesn't throw
void b_else_if_not(const Argm& argm, Error_list& exceptions) {
  try { if (!gc_state.exception_thrown)
    if_core(argm, exceptions, gc_state, false, false);}
  catch (Exception exception) {
    gc_state.exception_thrown = true;
    throw exception;}}

// run argfunction if successful_condition is false
void b_else(const Argm& argm, Error_list& exceptions) {
  try {
    if (gc_state.exception_thrown) gc_state.exception_thrown = false;
    else if_core(argm, exceptions, gc_state, false, true);
    gc_state.successful_condition = gc_state.in_if_block = false;}
  catch (Exception exception) {
    gc_state.successful_condition = gc_state.in_if_block = false;
    throw exception;}}

// prints a list of all internal functions
void b_internal_functions(const Argm& argm, Error_list& exceptions) {
  for (int i = 1; i < Argm::Exception_count; ++i)
    argm.output <<Argm::exception_names[i] <<"\n";}

// throws .false if the input stream is not the default_stream
void b_is_default_input(const Argm& argm, Error_list& exceptions) {
  if (!argm.input.is_default())
    exceptions.add_error(Exception(Argm::False, "is_default_input"));}

// throws .false if the output stream is not the default_stream
void b_is_default_output(const Argm& argm, Error_list& exceptions) {
  if (!argm.output.is_default())
    exceptions.add_error(Exception(Argm::False, "is_default_output"));}

// throws .false if the error stream is not the default_stream
void b_is_default_error(const Argm& argm, Error_list& exceptions) {
  if (!argm.error.is_default())
    exceptions.add_error(Exception(Argm::False, "is_default_error"));}

// print the last exception that was thrown by this function
void b_last_exception(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->last_exception() <<"\n";
    argm.output.flush();}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// print the number of times that the executable in the executable map with
// key $1 has been run
void b_last_execution_time(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->last_execution_time();
    argm.output.flush();}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// print the environment that the shell started in
void b_list_environment(const Argm& argm, Error_list& exceptions) {
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos) {
      if (i != environ) argm.output <<" ";
      argm.output <<word_from_value(
        word_from_value(src.substr(0, div)) + " " +
        word_from_value(src.substr(div+1)));}}}

// prints the binaries that have been called, and all builtins and functions
void b_list_executables(const Argm& argm, Error_list& exceptions) {
  for (auto i: executable_map)
    argm.output <<(i == *executable_map.begin()? "": " ") <<i.first;}

// prints all variables in the local variable map
void b_list_locals(const Argm& argm, Error_list& exceptions) {
  for (auto i: *argm.parent_map())
    argm.output <<(i.first == argm.parent_map()->begin()->first? "": " ")
                <<i.first;
  argm.locals_listed();}

// add a variable to the variable map until the enclosing function terminates
void b_local(const Argm& argm, Error_list& exceptions) {
  argm.local(argm[1], argm[2]);}

// add, but don't define a  variable until the enclosing function terminates
void b_local_declare(const Argm& argm, Error_list& exceptions) {
  for (unsigned i=1; i<argm.argc(); ++i)
    argm.local_declare(argm[i], exceptions);}

// list the files specified by the arguments if they exist
void b_ls(const Argm& argm, Error_list& exceptions) {
  struct stat sb;
  bool found = false;
  for (auto i: argm) if (!stat(i.c_str(), &sb)) {
    argm.output <<i <<"\n";
    found = true;}
  argm.output.flush();
  if (!found) {
    std::string tried(argm[1]);
    for (auto i: argm.subrange(2)) tried += " " + i;
    exceptions.add_error(Exception(Argm::File_not_found, tried));}}

// ignore arguments, argfunctions, and then do nothing
void b_nop(const Argm& argm, Error_list& exceptions) {}

// print the process id of the shell
void b_getpid(const Argm& argm, Error_list& exceptions) {
  argm.output <<(unsigned) getpid();}

// print the parent process id of the shell
void b_getppid(const Argm& argm, Error_list& exceptions) {
  argm.output <<(unsigned) getppid();}

// add the given exception as a replacement for the current one (if nothing
// afterwards fails)
void b_replace_exception(const Argm& argm, Error_list& exceptions) {
  if (!global_stack.in_exception_handler())
    throw Exception(Argm::Not_catching_exception);
  Argm new_exception(argm.subrange(1), argm.argfunction(),
                     Variable_map::global_map,
                     argm.input, argm.output.child_stream(), argm.error);
  exceptions.replace_error(new_exception);}

// remove executable with name $1 from executable map
void b_rm_executable(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map());
  Base_executable *e = executable_map.find_second(lookup);
  if (dynamic_cast<Builtin*>(e))
    exceptions.add_error(Exception(Argm::Illegal_function_name, argm[1]));
  else if (!executable_map.erase(*(argm.begin()+1)))
    exceptions.add_error(Exception(Argm::Function_not_found, argm[1]));
  else;} // successfully removed executable

// run the argfunction having set local variables according to the given
// prototype
void b_scope(const Argm& argm, Error_list& exceptions) {
  Argv prototype_argv;
  tokenize_words(argm[argm.argc()-1], std::back_inserter(prototype_argv));
  Argm invoking_argm(argm.subrange(0, 1), nullptr, argm.parent_map(),
                     argm.input, argm.output, argm.error);
  (*argm.argfunction()).prototype_execute(invoking_argm, prototype_argv,
                                                    exceptions);}

// modify variable $1 as a selection according to $2
void b_selection_set(const Argm& argm, Error_list& exceptions) {
  std::list<Entry_pattern> focus;
  str_to_entry_pattern_list(argm.get_var(argm[1]), focus);
  std::string change = *(argm.begin()+2);
  for (auto i: argm.subrange(3)) change += ' ' + i;
  str_to_entry_pattern_list(change, focus);
  argm.set_var(argm[1], entry_pattern_list_to_str(focus.begin(),focus.end()));}

// set variable $1 to $*2
// throws exception if the variable does not exist
void b_set(const Argm& argm, Error_list& exceptions) {
  if (isargvar(argm[1])) throw Exception(Argm::Illegal_variable_name, argm[1]);
  std::string dest("");
  for (auto i: argm.subrange(2, 1)) dest += i + ' ';
  dest += argm.back();
  argm.set_var(argm[1], dest);}

// Set the configurable message for fallback_handler
// When this is given a prototype, just use the collected args directly
void b_set_fallback_message(const Argm& argm, Error_list& exceptions) {
  fallback_message = "";
  for (auto i: argm.subrange(1, 1)) fallback_message += i + " ";
  fallback_message += argm.back();}

// Set the number of exceptions that can be thrown inside .collect_errors_*
// before they exit early
void b_set_max_collectible_exceptions(const Argm& argm, Error_list& exceptions){
  int val = my_strtoi(argm[1], 1, INT_MAX, exceptions);
  if (!global_stack.unwind_stack()) global_stack.max_collect = val;}

// Set the number of exceptions that can be thrown by catch blocks after
// max_collectible_exceptions have already been thrown
void b_set_max_extra_exceptions(const Argm& argm, Error_list& exceptions) {
  int val = my_strtoi(argm[1], 0, INT_MAX, exceptions);
  if (!global_stack.unwind_stack()) global_stack.max_extra = val;}

// Set the maximum number of nesting levels where functions call functions
// before completing.
void b_set_max_nesting(const Argm& argm, Error_list& exceptions) {
  int val = my_strtoi(argm[1], 0, INT_MAX);
  if (!global_stack.unwind_stack()) global_stack.max_nesting = val;}

// run the first argument as if it was a script, passing additional arguments
// to that script
void b_source(const Argm& argm, Error_list& exceptions) {
  struct stat sb;
  if (stat(argm[1].c_str(), &sb))
    throw Exception(Argm::File_open_failure, argm[1]);
  if (!(sb.st_mode & S_IXUSR)) throw Exception(Argm::Not_executable, argm[1]);
  Rwsh_istream_p src(new File_istream(argm[1]), false, false);
  Command_stream command_stream(src, false);
  Command_block block;
  Prototype prototype(Argv{"--", "[argv", "...]"});
  Variable_map locals(argm.parent_map());
  prototype.arg_to_param(argm.subrange(1), locals, exceptions);
  Argm script_arg(argm.subrange(1), nullptr, &locals,
                  argm.input, argm.output.child_stream(), argm.error);
  try {
    while (!command_stream.fail() && !global_stack.unwind_stack()) {
      command_stream.getline(block, exceptions);
      if (command_stream.fail()) break;
      block.execute(script_arg, exceptions);}
    prototype.unused_var_check(&locals, exceptions);}
  catch (Exception error) {
    prototype.unused_var_check(&locals, exceptions);
    throw error;}}

// run the argument function once with each command in the specified function
// invocation
void b_stepwise(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map(),
                argm.input, argm.output.child_stream(), argm.error);
  Base_executable* e = executable_map.find_second(lookup);
  if (!e) throw Exception(Argm::Function_not_found, argm[1]);
  Function* f = dynamic_cast<Function*>(e);
  if (!f) return; //throw Exception(Argm::Not_a_function, argm[1]);
  // this must be caught and handled to use .stepwise recursively
  Variable_map locals(lookup.parent_map());
  f->arg_to_param(lookup.argv(), locals, exceptions);
  if (global_stack.unwind_stack()) return;
  Argm params(lookup.argv(), lookup.argfunction(), &locals,
              lookup.input, lookup.output, lookup.error);
  for (auto j: f->body) {
    Argm body_i(j.interpret(params, exceptions));
    if (global_stack.unwind_stack()) break;
    Argm body(".mapped_argfunction", body_i.argv(), nullptr,
              body_i.parent_map(), body_i.input, body_i.output, body_i.error);
    (*argm.argfunction())(body, exceptions);
    (void) global_stack.remove_exceptions(".continue", exceptions);
    if (global_stack.remove_exceptions(".break", exceptions) ||
        global_stack.unwind_stack()) break;}
  f->unused_var_check(&locals, exceptions);}

// run the argfunction and store its output in the variable $1
void b_store_output(const Argm& argm, Error_list& exceptions) {
  if (isargvar(argm[1])) throw Exception(Argm::Illegal_variable_name, argm[1]);
  Substitution_stream text;
  Argm mapped_argm(argm.parent_map(),
                   argm.input, text.child_stream(), argm.error);
  mapped_argm.push_back(".mapped_argfunction");
  (*argm.argfunction())(mapped_argm, exceptions);
  if (!global_stack.unwind_stack()) argm.set_var(argm[1], text.value());}

// throws .false if the key $1 isn't an executable in the executable map 
void b_test_executable_exists(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  if (lookup[0] == ".argfunction") lookup[0] = ".mapped_argfunction";
  if (!executable_map.find_second(lookup))
    exceptions.add_error(Exception(Argm::False,
                                   "executable exists: " + argm[1]));}

// throws .false if none of the files specified by the arguments exist
void b_test_file_exists(const Argm& argm, Error_list& exceptions) {
  struct stat sb;
  for (auto i: argm.subrange(1)) if (!stat(i.c_str(), &sb)) return;
  std::string err_val;
  for (auto i: argm.subrange(1)) err_val += " -e " + i;
  exceptions.add_error(Exception(Argm::False, err_val));}

// throws .false  if two strings convert to a doubles and first is not greater
void b_test_greater(const Argm& argm, Error_list& exceptions) {
  double lhs = my_strtod(argm[1], exceptions),
         rhs = my_strtod(argm[2], exceptions);
  if (global_stack.unwind_stack()) return;
  if (lhs <= rhs)
    exceptions.add_error(Exception(Argm::False, argm[1] +" -gt "+ argm[2]));}

// throws .false  if the first string isn't repeated in subsequent arguments
void b_test_in(const Argm& argm, Error_list& exceptions) {
  for (auto j: argm.subrange(2)) if (argm[1] == j) return;
  auto err_val = argm[1] + " in";
  for (auto j: argm.subrange(2)) err_val += " " + j;
  exceptions.add_error(Exception(Argm::False, err_val));}

// throws .false if the string doesn't convert to a number
void b_test_is_number(const Argm& argm, Error_list& exceptions) {
  try {(void) my_strtod(argm[1]);}
  catch (E_generic) {exceptions.add_error(Exception(Argm::False,
                        "is_number " + argm[1] + " - generic"));}
  catch (E_nan) {exceptions.add_error(Exception(Argm::False,
                        "is_number " + argm[1] + " - NaN"));}
  catch (E_range) {exceptions.add_error(Exception(Argm::False,
                        "is_number " + argm[1] + " - range"));}}

// throws .false if two strings convert to doubles and first isn't less
void b_test_less(const Argm& argm, Error_list& exceptions) {
  double lhs = my_strtod(argm[1], exceptions),
         rhs = my_strtod(argm[2], exceptions);
  if (global_stack.unwind_stack()) return;
  if (lhs >= rhs)
    exceptions.add_error(Exception(Argm::False, argm[1] +" -lt "+ argm[2]));}

// throws .false if the string is empty
void b_test_not_empty(const Argm& argm, Error_list& exceptions) {
  if (!argm[1].length())
    exceptions.add_error(Exception(Argm::False, " -n " + argm[1]));}

// throw .false if the two strings convert to doubles and are not equal
void b_test_number_equal(const Argm& argm, Error_list& exceptions) {
  double lhs = my_strtod(argm[1], exceptions),
         rhs = my_strtod(argm[2], exceptions);
  if (global_stack.unwind_stack()) return;
  if (lhs != rhs)
    exceptions.add_error(Exception(Argm::False, argm[1] +" -eq "+ argm[2]));}

// throws .false if the two strings aren't the same
void b_test_string_equal(const Argm& argm, Error_list& exceptions) {
  if (argm[1] != argm[2])
    exceptions.add_error(Exception(Argm::False, argm[1] + " == " + argm[2]));}

// throws .false if the two strings are the same
void b_test_string_unequal(const Argm& argm, Error_list& exceptions) {
  if (argm[1] == argm[2])
    exceptions.add_error(Exception(Argm::False, argm[1] + " != " + argm[2]));}

// throw the remaining arguments as an exception
void b_throw(const Argm& argm, Error_list& exceptions) {
  Argm new_exception(argm.subrange(1), argm.argfunction(),
                     Variable_map::global_map, default_input, default_output,
                     default_error);
  exceptions.add_error(new_exception);}

// enable readline if disabled, disable if enabled
void b_toggle_readline(const Argm& argm, Error_list& exceptions) {
  readline_enabled = ! readline_enabled;}

// print the number of times that the executable in the executable map with
// key $1 has been run
void b_total_execution_time(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    struct timeval val = focus->total_execution_time();
    argm.output <<val;
    argm.output.flush();}
  else throw Exception(Argm::Function_not_found, argm[1]);}

// run the handler for specified exceptions
void b_try_catch_recursive(const Argm& argm, Error_list& exceptions) {
  Argm mapped_argm(argm.parent_map(),
                   argm.input, argm.output.child_stream(), argm.error);
  mapped_argm.push_back(".try_catch_recursive(body)");
  (*argm.argfunction())(mapped_argm, exceptions);
  if (global_stack.unwind_stack()) global_stack.catch_blocks(argm,exceptions);}

// print the type of executable with name $1 from executable map
void b_type(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  if (lookup[0] == ".argfunction") lookup[0] = ".mapped_argfunction";
  Base_executable *e = executable_map.find_second(lookup);
  if (!e) exceptions.add_error(Exception(Argm::Function_not_found, argm[1]));
  else if (dynamic_cast<Function*>(e)) argm.output <<"function\n";
  else if (dynamic_cast<Binary*>(e))   argm.output <<"file\n";
  else if (dynamic_cast<Builtin*>(e))  argm.output <<"builtin\n";
  else if (dynamic_cast<Command_block*>(e))  argm.output <<"argfunction\n";
  else std::abort();} // unexpected type of executable

// removes the given variable from the variable map. you could be really
// pedantic and throw an .undefined_variable if it doesn't exist, but the
// fact is that the requested state (one where this variable isn't set) is
// already the case, so it's hard to say what you're protecting people from.
void b_unset(const Argm& argm, Error_list& exceptions) {
  argm.unset_var(argm[1]);}

namespace {double sleep_requested = 0.0;}

// sleep for the specified number of microseconds
void b_usleep(const Argm& argm, Error_list& exceptions) {
  int usec = my_strtoi(argm[1], 0, INT_MAX, exceptions);
  if (global_stack.unwind_stack()) return;
  sleep_requested += usec / 1000000.0;
  const long mega = (long)1E6;
  struct timespec delay = {usec / mega, (usec % mega) * 1000};
  if (!nanosleep(&delay, nullptr)) return;
  else if (errno == EINTR)                                      //not tested
    throw Exception(Argm::Interrupted_sleep);
  else throw Exception(Argm::Internal_error,                    //not tested
                       "nanosleep failed with code", errno);}

// print the average number of microseconds longer that .usleep takes than it
// is requested to take.
void b_usleep_overhead(const Argm& argm, Error_list& exceptions) {
  Argm usleep_argm(".usleep", Argv(), nullptr, Variable_map::global_map,
                   default_input, default_output, default_error);
  Base_executable* focus = executable_map.find_second(Argm(usleep_argm));
  unsigned count = focus->execution_count();
  if (!count) throw Exception(Argm::Not_a_number, "");
  struct timeval slept = focus->total_execution_time();
  double total = slept.tv_sec + slept.tv_usec/1000000.0 - sleep_requested;
  argm.output <<(total/count);}
  
// add the specified value to the specified variable
void b_var_add(const Argm& argm, Error_list& exceptions) {
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term = my_strtod(var_str, exceptions);
  double const_term = my_strtod(argm[2], exceptions);
  if (global_stack.unwind_stack()) return;
  double sum = var_term + const_term;
  if (sum == std::numeric_limits<double>::infinity() ||
      sum == -std::numeric_limits<double>::infinity())
    return exceptions.add_error(Exception(Argm::Result_range,var_str,argm[2]));
  std::string result = my_dtostr(sum);
  if (result == my_dtostr(var_term) && const_term != 0.0)
    return exceptions.add_error(Exception(Argm::Epsilon, var_str, argm[2]));
  argm.set_var(argm[1], result);}

// divide the specified variable by the specified value
void b_var_divide(const Argm& argm, Error_list& exceptions) {
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term = my_strtod(var_str, exceptions);
  double const_term = my_strtod(argm[2], exceptions);
  if (const_term == 0)
    exceptions.add_error(Exception(Argm::Divide_by_zero, var_str));
  if (global_stack.unwind_stack()) return;
  double quotient = var_term / const_term;
  if (quotient == 0 && var_term != 0)
    return exceptions.add_error(Exception(Argm::Result_range,var_str,argm[2]));
  std::string result = my_dtostr(quotient);
  if (result == my_dtostr(var_term) && var_term != 0.0 && const_term != 1.0)
    return exceptions.add_error(Exception(Argm::Epsilon, var_str, argm[2]));
  argm.set_var(argm[1], result);}

// divide the specified variable by the specified value
void b_var_multiply(const Argm& argm, Error_list& exceptions) {
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term = my_strtod(var_str, exceptions);
  double const_term = my_strtod(argm[2], exceptions);
  if (global_stack.unwind_stack()) return;
  double product = var_term * const_term;
  if (product == std::numeric_limits<double>::infinity() ||
      product == -std::numeric_limits<double>::infinity())
    return exceptions.add_error(Exception(Argm::Result_range,var_str,argm[2]));
  std::string result = my_dtostr(product);
  if (result == my_dtostr(var_term) && var_term != 0.0 && const_term != 1.0)
    return exceptions.add_error(Exception(Argm::Epsilon, var_str, argm[2]));
  argm.set_var(argm[1], result);}

// subtract the specified value from the specified variable
void b_var_subtract(const Argm& argm, Error_list& exceptions) {
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term = my_strtod(var_str, exceptions);
  double const_term = my_strtod(argm[2], exceptions);
  if (global_stack.unwind_stack()) return;
  double difference = var_term - const_term;
  if (difference >= DBL_MAX || difference <= -DBL_MAX)
    return exceptions.add_error(Exception(Argm::Result_range,var_str,argm[2]));
  std::string result = my_dtostr(difference);
  if (result == my_dtostr(var_term) && const_term != 0.0)
    return exceptions.add_error(Exception(Argm::Epsilon, var_str, argm[2]));
  argm.set_var(argm[1], result);}

// throw .false if none of the specified variables exist
void b_var_exists(const Argm& argm, Error_list& exceptions) {
  bool any_exist = false;
  // need to go through the whole set to mark all as checked
  for (auto i: argm.subrange(1)) if (argm.var_exists(i)) any_exist = true;
  if (any_exist) return;
  std::string err_val ("var exists:");
  for (auto i: argm.subrange(1)) err_val += " " + i;
  exceptions.add_error(Exception(Argm::False, err_val));}

static const std::string version_str("0.3+");

// write to standard output the version of rwsh
void b_version(const Argm& argm, Error_list& exceptions) {
  argm.output <<version_str;}

// throws Version_incompatibles if the given version string is not compatible
// with the version of this shell
void b_version_compatible(const Argm& argm, Error_list& exceptions) {
  if (argm[1] != version_str)
    throw Exception(Argm::Version_incompatible, argm[1]);}

// prints the total amount of time the shell has spent in wait() syscall
void b_waiting_for_binary(const Argm& argm, Error_list& exceptions) {
  argm.output <<rwsh_clock.waiting_for_binary();
  argm.output.flush();}

// prints the total amount of time that has passed and the shell has not been
// waiting for other processes or the user
void b_waiting_for_shell(const Argm& argm, Error_list& exceptions) {
  argm.output <<rwsh_clock.waiting_for_shell();
  argm.output.flush();}

// prints the total amount of time the shell has been waiting for user input
void b_waiting_for_user(const Argm& argm, Error_list& exceptions) {
  argm.output <<rwsh_clock.waiting_for_user();
  argm.output.flush();}

// print the string corresponding to the executable in the executable map with
// key $1
void b_whence_function(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), argm.argfunction(), argm.parent_map());
  if (lookup[0] == ".argfunction") lookup[0] = ".mapped_argfunction";
  Base_executable* focus = executable_map.find_second(lookup);
  if (focus) {
    argm.output <<focus->str() <<"\n";
    argm.output.flush();}
  else exceptions.add_error(Exception(Argm::Function_not_found, argm[1]));}

// if the filename has a leading dot, then check in current directory
// otherwise find the binary in $2 with filename $1
void b_which_path(const Argm& argm, Error_list& exceptions) {
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
      return;}}
  exceptions.add_error(                          // executable does not exist
                       Exception(Argm::Binary_not_found, argm[1], argm[2]));}

// run the argfunction each time that the arguments don't throw an exception
void b_while(const Argm& argm, Error_list& exceptions) {
  Argm lookup(argm.subrange(1), nullptr, argm.parent_map(),
              argm.input, argm.output.child_stream(), argm.error);
  while (executable_map.run_condition(lookup, exceptions)) {
    Argm mapped_argm(argm.parent_map(), argm.input, argm.output.child_stream(),
                     argm.error);
    mapped_argm.push_back(".mapped_argfunction");
    (*argm.argfunction())(mapped_argm, exceptions);
    (void) global_stack.remove_exceptions(".continue", exceptions);
    if (global_stack.remove_exceptions(".break", exceptions) ||
        global_stack.unwind_stack()) return;}}
