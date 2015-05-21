// The functions that implement each of the builtin executables
//
// Copyright (C) 2006-2015 Samuel Newbold

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <list>
#include <limits>
#include <map>
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

#include "argm.h"
#include "arg_script.h"
#include "builtin.h"
#include "clock.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "read_dir.cc"
#include "rwshlib.h"
#include "selection.h"
#include "substitution_stream.h"
#include "tokenize.cc"
#include "variable_map.h"

#include "argm_star_var.cc"

// change the current directory to the one given
// returns the error returned from chdir
int b_cd(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  errno = 0;
  int ret = chdir(argm[1].c_str());
  if (!ret) return 0;
  else if (errno == ENOENT) ret = 1;
  else if (errno == ENOTDIR) ret = 2; 
  else ret = 3; 
  errno = 0;
  return ret;}

// echo arguments to standard output without space separation
int b_combine(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  for (Argm::const_iterator i = argm.begin()+1; i != argm.end(); ++i)
    argm.output <<*i;
  argm.output.flush();
  return 0;}

// echo arguments to standard output separated by space
int b_echo(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  for (Argm::const_iterator i = argm.begin()+1; i != argm.end()-1; ++i)
    argm.output <<*i <<" ";
  argm.output <<argm.back();
  argm.output.flush();
  return 0;}

#include <iostream>
// replace the shell with the given binary
int b_exec(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  int input = argm.input.fd(),
      output = argm.output.fd(),
      error = argm.error.fd();
  if (dup2(input, 0) < 0) std::cerr <<"dup2 didn't like changing input\n";
  if (dup2(output, 1) < 0) std::cerr <<"dup2 didn't like changing output\n";
  if (dup2(error, 2) < 0) std::cerr <<"dup2 didn't like changing error\n";
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), 
                default_input, default_output, default_error);
  Old_argv old_argv(lookup);
  char **env = argm.export_env();
  int ret = execve(lookup[0].c_str(), old_argv.argv(), env);
  Signal_argm error_argm(Argm::Binary_not_found, argm[0]); 
  executable_map.run(error_argm);
  return ret;}

// exit the shell
int b_exit(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  Variable_map::exit_requested = true;
  return 0;}

// run the argfunction for each argument, passing that value as the argument
// returns the value returned by the argfunction
int b_for(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  int ret = -1;
  Argm body(argm.input, argm.output, argm.error);
  body.push_back("rwsh.mapped_argfunction");
  body.push_back("");
  for (Argm::const_iterator i = ++argm.begin(); i != argm.end(); ++i) {
    if (argm.argfunction()) {
      body[1] = *i;
      ret  = (*argm.argfunction())(body);
      if (Executable::unwind_stack()) return -1;}
    else ret = 0;}
  return ret;}

// run the argfunction for line of input, passing that line as the argm
// returns the value returned by the argfunction
int b_for_each_line(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  int ret = -1;
  while(!argm.input.fail()) { 
    std::string line;
    // shouldn't interfere with input being consumed by this builtin
    Argm body(default_input, argm.output, argm.error);
    body.push_back("rwsh.mapped_argfunction");
    argm.input.getline(line);
    if (argm.input.fail() && !line.size()) break;
    tokenize(line, std::back_inserter(body), 
             std::bind2nd(std::equal_to<char>(), ' '));
    ret = (*argm.argfunction())(body);}
  return ret;}

#include "plumber.h"
int b_fork(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  // argfunction optional
  int ret = 0;
  if (!fork()) {  
    plumber.after_fork();
    Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(),
                  argm.input, argm.output.child_stream(), argm.error);
    ret = executable_map.run(lookup);
    exit(ret);}
  else plumber.wait(&ret);
  return ret;}

// add argfunction to executable map with name $1
int b_function(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  else if (is_binary_name(argm[1])) return 1;
  Argm lookup(argm.begin()+1, argm.end(), NULL, 
                default_input, default_output, default_error);
  Executable *e = executable_map.find(lookup);
  if (e && dynamic_cast<Builtin*>(e)) return 2;
  else if (is_argfunction_name(argm[1])) return 3;
  else if (!argm.argfunction()) {
    return 4 * !executable_map.erase(*(argm.begin()+1));}
  else {
    executable_map.set(new Function(argm[1], argm.end(), argm.end(), true,
                                    argm.argfunction()->script));
    return 0;}}

// add argfunction to executable map with name $1 and arguments $*2
// the arguments must include all options that can be passed to this function
int b_function_all_options(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  else if (is_binary_name(argm[1])) return 1;
  Argm lookup(argm.begin()+1, argm.begin()+2, NULL, 
                default_input, default_output, default_error);
  Executable *e = executable_map.find(lookup);
  if (e && dynamic_cast<Builtin*>(e)) return 2;
  else if (is_argfunction_name(argm[1])) return 3;
  else if (!argm.argfunction()) {
    return 4 * !executable_map.erase(*(argm.begin()+1));}
  else {
    Function *focus = new Function(argm[1], argm.begin()+2, argm.end(), false,
                                   argm.argfunction()->script);
    executable_map.set(focus);
    return 0;}}

// add a variable to the variable map that will remain after the enclosing
// function terminates
int b_global(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  else return argm.global_var(argm[1], argm[2]);}

namespace {
int if_core(const Argm& argm, bool logic) {
  Argm lookup(argm.begin()+1, argm.end(), 0,
                argm.input, argm.output.child_stream(), argm.error);
  if (logic == !executable_map.run(lookup)) {
    if (Executable::unwind_stack()) return -1;
    argm.unset_var("IF_TEST");
    int ret;
    if (argm.argfunction()) {
      Argm mapped_argm(argm.input, argm.output.child_stream(), argm.error);
      mapped_argm.push_back("rwsh.mapped_argfunction");
      ret = (*argm.argfunction())(mapped_argm);}
    else ret = 0;
    if (argm.var_exists("IF_TEST")) {
      argm.unset_var("IF_TEST");
      throw Signal_argm(Argm::Bad_if_nest);}
    else argm.global_var("IF_TEST", "true");
    return ret;}
  else return 0;}
}

// run argfunction if $* returns true
// returns the value returned by the argfunction
int b_if(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  else if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  else if (!argm.var_exists("IF_TEST")) {
    argm.global_var("IF_TEST", "false"); 
    return if_core(argm, true);}
  else {
    argm.unset_var("IF_TEST");
    throw Signal_argm(Argm::If_before_else);}}

// run argfunction if IF_TEST is false and $* returns true
// returns the value returned by the argfunction
int b_else_if(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  else if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  else if (!argm.var_exists("IF_TEST"))
    throw Signal_argm(Argm::Else_without_if);
  else if (argm.get_var("IF_TEST") == "true") return dollar_question;
  else if (argm.get_var("IF_TEST") == "false") return if_core(argm, true);
  else throw Signal_argm(Argm::Else_without_if);}

// run argfunction if IF_TEST is false and $* returns false
// returns the value returned by the argfunction
int b_else_if_not(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  else if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  else if (!argm.var_exists("IF_TEST"))
    throw Signal_argm(Argm::Else_without_if);
  else if (argm.get_var("IF_TEST") == "true") return dollar_question;
  else if (argm.get_var("IF_TEST") == "false") return if_core(argm, false);
  else throw Signal_argm(Argm::Else_without_if);}

// run argfunction if IF_TEST is false 
// returns the value returned by the argfunction
int b_else(const Argm& argm) {
  int ret;
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  else if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  else if (!argm.var_exists("IF_TEST")) {
    argm.unset_var("IF_TEST");
    throw Signal_argm(Argm::Else_without_if);}
  else if (argm.get_var("IF_TEST") == "true") ret = dollar_question;
  else if (argm.get_var("IF_TEST") == "false") 
    if (argm.argfunction()) {
      argm.unset_var("IF_TEST");
      Argm mapped_argm(argm.input, argm.output.child_stream(), argm.error);
      mapped_argm.push_back("rwsh.mapped_argfunction");
      (*argm.argfunction())(mapped_argm);
      if (argm.var_exists("IF_TEST")) {
        argm.unset_var("IF_TEST");
        throw Signal_argm(Argm::Bad_if_nest);}
      else ret = dollar_question;}
    else ret = 0;
  else {
    argm.unset_var("IF_TEST");
    throw Signal_argm(Argm::Else_without_if);}
  argm.unset_var("IF_TEST");
  return ret;}

// import the external environment into the variable map, overwriting variables
// that already exist
int b_importenv_overwrite(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos) 
      if (argm.var_exists(src.substr(0, div)))
        argm.set_var(src.substr(0, div), src.substr(div+1));
      else argm.global_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// import the external environment into the variable map, preserving variables
// that already exist
int b_importenv_preserve(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos && !argm.var_exists(src.substr(0, div))) 
      argm.global_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// prints a list of all internal functions
int b_internal_functions(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  argm.output <<Argm::signal_names[1];
  for (int i = 2; i < Argm::Signal_count; ++i)
    argm.output <<"\n" <<Argm::signal_names[i];
  return 0;}

// returns one if the input stream is not the default_stream
int b_is_default_input(const Argm& argm) {
  if(argm.argc() != 1) throw Signal_argm(Argm::Argument_count, argm.argc()-1,0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  return !argm.input.is_default();}

// returns one if the output stream is not the default_stream
int b_is_default_output(const Argm& argm) {
  if(argm.argc() != 1) throw Signal_argm(Argm::Argument_count, argm.argc()-1,0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  return !argm.output.is_default();}

// returns one if the error stream is not the default_stream
int b_is_default_error(const Argm& argm) {
  if(argm.argc() != 1) throw Signal_argm(Argm::Argument_count, argm.argc()-1,0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  return !argm.error.is_default();}

// list the files specified by the arguments if they exist
int b_ls(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  struct stat sb;
  int ret = 1;
  for (Argm::const_iterator i=argm.begin(); i != argm.end(); ++i) 
    if (!stat(i->c_str(), &sb)) {
      argm.output <<*i <<"\n";
      ret = 0;}
  argm.output.flush();
  return ret;}

// ignore arguments, argfunctions, and then do nothing
int b_nop(const Argm& argm) {return dollar_question;}

// return the value given by the argument
int b_return(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  try {return my_strtoi(argm[1]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[1]);}}

// modify variable $1 as a selection according to $2
int b_selection_set(const Argm& argm) {
  if (argm.argc() < 3) throw Signal_argm(Argm::Argument_count, argm.argc()-1,2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
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
int b_set(const Argm& argm) {
  if (argm.argc() < 3) throw Signal_argm(Argm::Argument_count, argm.argc()-1,2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  if (isargvar(argm[1]) || argm[1] == "IF_TEST") return 2;
  std::string dest("");
  for (Argm::const_iterator i = argm.begin()+2; i != argm.end()-1; ++i) 
    dest += *i + ' ';
  dest += argm.back();
  return argm.set_var(argm[1], dest);}

// run the handler for specified signals
int b_signal_handler(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  Argm mapped_argm(argm.input, argm.output.child_stream(), argm.error);
  mapped_argm.push_back("rwsh.mapped_argfunction");
  int ret = (*argm.argfunction())(mapped_argm);
  if (Executable::unwind_stack()) {
    for (Argm::const_iterator i = argm.begin() +1; i != argm.end(); ++i) 
      if (*i == Argm::signal_names[Executable::unwind_stack()]) {
        Executable::call_stack.pop_back(); // get rid of rwsh.mapped_argfunction
        Executable::signal_handler();}
    return -1;}
  return ret;}

// run the first argument as if it was a script, passing additional arguments
// to that script
// returns last return value from script, -1 if empty
int b_source(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  struct stat sb;
  if (stat(argm[1].c_str(), &sb)) 
    throw Signal_argm(Argm::File_open_failure, argm[1]);
  if (!(sb.st_mode & S_IXUSR)) throw Signal_argm(Argm::Not_executable, argm[1]);
  std::ifstream src(argm[1].c_str(), std::ios_base::in);
  Argm script_arg(argm.begin()+1, argm.end(), 0, argm.input,
                    argm.output.child_stream(), argm.error);
  Command_stream command_stream(src, false);
  Arg_script script("", 0);
  int ret = -1;
  while (command_stream && !Executable::unwind_stack()) {
    try {
      if (!(command_stream >> script)) break;
      Argm command(script.interpret(script_arg));
      ret = executable_map.run(command);}
    catch (Signal_argm exception) {
      ret = executable_map.run(exception);}}
  return ret;}

// run the argument function once with each command in the specified function
// invocation
int b_stepwise(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  Argm lookup(argm.begin()+1, argm.end(), 0,
                argm.input, argm.output.child_stream(), argm.error);
  Executable* e = executable_map.find(lookup);
  if (!e) return 1;  // executable not found
  Function* f = dynamic_cast<Function*>(e);
  if (!f) return 2; // the named executable is not a function
  if (f->increment_nesting(lookup)) return dollar_question;
  int ret = -1;
  for (Function::const_iterator i = f->script.begin(); 
       i != f->script.end(); ++i) {
    Argm body_i(i->interpret(lookup));
    Argm body("rwsh.mapped_argfunction", body_i.begin(), body_i.end(), 0,
                body_i.input, body_i.output, body_i.error);
    ret  = (*argm.argfunction())(body);
    if (Executable::unwind_stack()) {
      ret = -1;
      break;}}
  if (f->decrement_nesting(lookup)) ret = dollar_question;
  return ret;} // last return value from argfunction

// run the argfunction and store its output in the variable $1
// returns the last return from the argfunction
int b_store_output(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  if (isargvar(argm[1]) || argm[1] == "IF_TEST") return 2;
  Substitution_stream text;
  Argm mapped_argm(argm.input, text.child_stream(), argm.error);
  mapped_argm.push_back("rwsh.mapped_argfunction");
  int ret = (*argm.argfunction())(mapped_argm);
  if (Executable::unwind_stack()) return -1;
  if (ret) return ret;
  argm.set_var(argm[1], text.value());
  return 0;}

// return true if the two strings are the same
int b_test_string_equal(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  else return argm[1] != argm[2];} // C++ and shell have inverted logic

// return true if two strings convert to a doubles and first is greater
int b_test_greater(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  double lhs, rhs;
  try {lhs = my_strtod(argm[1]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[1]);}
  try {rhs = my_strtod(argm[2]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[2]);}
  return lhs <= rhs;} // C++ and shell have inverted logic

// return true if the string converts to a number
int b_test_is_number(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  try {
    (void) my_strtod(argm[1]);
    return 0;}
  catch (E_generic) {return 1;}
  catch (E_nan) {return 1;}
  catch (E_range) {return 2;}}

// return true if two strings convert to a doubles and first is less
int b_test_less(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  double lhs, rhs;
  try {lhs = my_strtod(argm[1]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[1]);}
  try {rhs = my_strtod(argm[2]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[2]);}
  return lhs >= rhs;} // C++ and shell have inverted logic

// return true if the string is not empty
int b_test_not_empty(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  else return !argm[1].length();} // C++ and shell have inverted logic

// return true if the two strings are different 
int b_test_string_unequal(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  else return argm[1] == argm[2];} // C++ and shell have inverted logic

// returns true if the two strings 
int b_test_number_equal(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  double lhs, rhs;
  try {lhs = my_strtod(argm[1]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[1]);}
  try {rhs = my_strtod(argm[2]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[2]);}
  return lhs != rhs;} // C++ and shell have inverted logic

// removes the given variable from the variable map. you could be really 
// pedantic and throw an rwsh.undefined_variable if it doesn't exist, but the
// fact is that the requested state (one where this variable isn't set) is
// already the case, so it's hard to say what you're protecting people from.
int b_unset(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  else return argm.unset_var(argm[1]);}

// sleep for the specified number of microseconds
int b_usleep(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  try {
    int delay = my_strtoi(argm[1], 0, INT_MAX);
    return usleep(delay);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[1]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[1]);}}
  
int b_var_add(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term;
  try {var_term = my_strtod(var_str);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, var_str);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, var_str);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, var_str);}
  double const_term;
  try {const_term = my_strtod(argm[2]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[2]);}
  double sum = var_term + const_term;
  if (sum == std::numeric_limits<double>::infinity() ||
      sum == -std::numeric_limits<double>::infinity())
    throw Signal_argm(Argm::Result_range, var_str, argm[2]);
  std::ostringstream tmp; 
  tmp <<sum;
  argm.set_var(argm[1], tmp.str());
  return 0;}

int b_var_subtract(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term;
  try {var_term = my_strtod(var_str);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, var_str);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, var_str);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, var_str);}
  double const_term;
  try {const_term = my_strtod(argm[2]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[2]);}
  double difference = var_term - const_term;
  if (difference == 1e309 || difference == -1e309)
    throw Signal_argm(Argm::Result_range, var_str, argm[2]);
  std::ostringstream tmp; 
  tmp <<difference;
  argm.set_var(argm[1], tmp.str());
  return 0;}

int b_var_divide(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  const std::string& var_str = argm.get_var(argm[1]);
  double var_term;
  try {var_term = my_strtod(var_str);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, var_str);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, var_str);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, var_str);}
  double const_term;
  try {const_term = my_strtod(argm[2]);}
  catch (E_generic) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_nan) {throw Signal_argm(Argm::Not_a_number, argm[2]);}
  catch (E_range) {throw Signal_argm(Argm::Input_range, argm[2]);}
  if (const_term == 0) throw Signal_argm(Argm::Divide_by_zero, var_str);
  double quotient = var_term / const_term;
  if (quotient == 0 && var_term != 0)
    throw Signal_argm(Argm::Result_range, var_str, argm[2]);
  std::ostringstream tmp; 
  tmp <<quotient;
  argm.set_var(argm[1], tmp.str());
  return 0;}

int b_var_exists(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  else return !argm.var_exists(argm[1]);}

static const std::string version_str("0.3+");

// write to standard output the version of rwsh
int b_version(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  argm.output <<version_str;
  return 0;}

// return true if the given version string is compatible with the version
// of this shell
int b_version_compatible(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  else if (argm[1] == version_str) return 0;
  else throw Signal_argm(Argm::Version_incompatible, argm[1]);}

// prints the total amount of time the shell has not been waiting for user input
int b_waiting_for_binary(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  argm.output <<rwsh_clock.waiting_for_binary();
  argm.output.flush();
  return 0;}

// prints the total amount of time that has passed and the shell has not been
// waiting for other processes or the user
int b_waiting_for_shell(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  argm.output <<rwsh_clock.waiting_for_shell();
  argm.output.flush();
  return 0;}

// prints the total amount of time the shell has been waiting for user input
int b_waiting_for_user(const Argm& argm) {
  if (argm.argc()!=1) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 0);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
  argm.output <<rwsh_clock.waiting_for_user();
  argm.output.flush();
  return 0;}

// print the string corresponding to the executable in the executable map with
// key $1
int b_which_executable(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->str();
    argm.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_which_execution_count(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->execution_count();
    argm.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_which_last_execution_time(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable* focus = executable_map.find(lookup);
  if (focus) {
    struct timeval val = focus->last_execution_time();
    argm.output <<val;
    argm.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int b_which_total_execution_time(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable* focus = executable_map.find(lookup);
  if (focus) {
    struct timeval val = focus->total_execution_time();
    argm.output <<val;
    argm.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// find the binary in $2 with filename $1
int b_which_path(const Argm& argm) {
  if (argm.argc()!=3) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 2);
  if (argm.argfunction()) throw Signal_argm(Argm::Excess_argfunction);
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
int b_which_return(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  Argm lookup(argm.begin()+1, argm.end(), 0,
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.mapped_argfunction" || 
            lookup[0] == "rwsh.argfunction") 
    return 2; // return values not stored for argfunctions
  Executable* focus = executable_map.find(lookup);
  if (focus) {
    argm.output <<focus->last_ret();
    argm.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// return true if there is an executable in the executable map with key $1
int b_which_test(const Argm& argm) {
  if (argm.argc()!=2) throw Signal_argm(Argm::Argument_count, argm.argc()-1, 1);
  Argm lookup(argm.begin()+1, argm.end(), argm.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  return !executable_map.find(lookup);}

// for each time that the arguments return true, run the argfunction
// returns the last return from the argfunction
int b_while(const Argm& argm) {
  if (argm.argc() < 2) throw Signal_argm(Argm::Argument_count, argm.argc()-1,1);
  if (!argm.argfunction()) throw Signal_argm(Argm::Missing_argfunction);
  int ret = -1;
  Argm lookup(argm.begin()+1, argm.end(), 0,
                argm.input, argm.output.child_stream(), argm.error);
  while (!executable_map.run(lookup)) {
    if (Executable::unwind_stack()) return -1;
    Argm mapped_argm(argm.input, argm.output.child_stream(), argm.error);
    mapped_argm.push_back("rwsh.mapped_argfunction");
    ret = (*argm.argfunction())(mapped_argm);
    if (Executable::unwind_stack()) return -1;}
  return ret;}

