// The functions that implement each of the builtin executables
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <list>
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

#include "argv.h"
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

#include "argv_star_var.cc"

// change the current directory to the one given
// returns the error returned from chdir
int cd_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  errno = 0;
  int ret = chdir(argv[1].c_str());
  if (!ret) argv.set_var("CWD", argv[1]);
  else if (errno == ENOENT) ret = 1;
  else if (errno == ENOTDIR) ret = 2; 
  else ret = 3; 
  errno = 0;
  return ret;}

// echo arguments to standard output without space separation
int combine_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  for (Argv_t::const_iterator i = argv.begin()+1; i != argv.end(); ++i)
    argv.output <<*i;
  argv.output.flush();
  return 0;}

// echo arguments to standard output separated by space
int echo_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  for (Argv_t::const_iterator i = argv.begin()+1; i != argv.end()-1; ++i)
    argv.output <<*i <<" ";
  argv.output <<argv.back();
  argv.output.flush();
  return 0;}

#include <iostream>
// replace the shell with the given binary
int exec_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  int input = argv.input.fd(),
      output = argv.output.fd(),
      error = argv.error.fd();
  if (dup2(input, 0) < 0) std::cerr <<"dup2 didn't like changing input\n";
  if (dup2(output, 1) < 0) std::cerr <<"dup2 didn't like changing output\n";
  if (dup2(error, 2) < 0) std::cerr <<"dup2 didn't like changing error\n";
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 
                default_input, default_output, default_error);
  Old_argv_t old_argv(lookup);
  char **env = argv.export_env();
  int ret = execve(lookup[0].c_str(), old_argv.argv(), env);
  Argv_t error_argv;
  error_argv.push_back("rwsh.binary_not_found");
  error_argv.push_back(argv[0]); 
  executable_map.run(error_argv);
  return ret;}

// exit the shell
int exit_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  Variable_map_t::exit_requested = true;
  return 0;}

// run the argfunction for each argument, passing that value as the argument
// returns the value returned by the argfunction
int for_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  int ret = -1;
  Argv_t body;
  body.input = argv.input;
  body.output = argv.output;
  body.error = argv.error;
  body.push_back("rwsh.mapped_argfunction");
  body.push_back("");
  for (Argv_t::const_iterator i = ++argv.begin(); i != argv.end(); ++i) {
    if (argv.argfunction()) {
      body[1] = *i;
      ret  = (*argv.argfunction())(body);
      if (Executable_t::unwind_stack()) return -1;}
    else ret = 0;}
  return ret;}

// run the argfunction for line of input, passing that line as the argv
// returns the value returned by the argfunction
int for_each_line_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  if (!argv.argfunction()) throw Missing_argfunction_t();
  int ret = -1;
  while(!argv.input.fail()) { 
    std::string line;
    Argv_t body;
    body.push_back("rwsh.mapped_argfunction");
    argv.input.getline(line);
    if (argv.input.fail() && !line.size()) break;
    tokenize(line, std::back_inserter(body), 
             std::bind2nd(std::equal_to<char>(), ' '));
    ret = (*argv.argfunction())(body);}
  return ret;}

#include "plumber.h"
int fork_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  int ret = 0;
  if (!fork()) {  
    plumber.after_fork();
    Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(),
                  argv.input, argv.output.child_stream(), argv.error);
    ret = executable_map.run(lookup);
    exit(ret);}
  else plumber.wait(&ret);
  return ret;}

// add argfunction to executable map with name $1
int function_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  else if (is_binary_name(argv[1])) return 1;
  Argv_t lookup(argv.begin()+1, argv.end(), NULL, 
                default_input, default_output, default_error);
  Executable_t *e = executable_map.find(lookup);
  if (e && dynamic_cast<Builtin_t*>(e)) return 2;
  else if (is_argfunction_name(argv[1])) return 3;
  else if (!argv.argfunction()) {
    return 4 * !executable_map.erase(*(argv.begin()+1));}
  else {
    executable_map.set(new Function_t(argv[1], argv.argfunction()));
    return 0;}}

// add a variable to the variable map that will remain after the enclosing
// function terminates
int global_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  else return argv.global_var(argv[1], argv[2]);}

namespace {
int if_core(const Argv_t& argv, bool logic) {
  Argv_t lookup(argv.begin()+1, argv.end(), 0,
                argv.input, argv.output.child_stream(), argv.error);
  if (logic == !executable_map.run(lookup)) {
    if (Executable_t::unwind_stack()) return -1;
    argv.unset_var("IF_TEST");
    int ret;
    if (argv.argfunction()) {
      Argv_t mapped_argv;
      mapped_argv.push_back("rwsh.mapped_argfunction");
      mapped_argv.output = argv.output.child_stream();
      ret = (*argv.argfunction())(mapped_argv);}
    else ret = 0;
    if (argv.var_exists("IF_TEST")) {
      argv.unset_var("IF_TEST");
      throw Bad_if_nest_t();}
    else argv.global_var("IF_TEST", "true");
    return ret;}
  else return 0;}
}

// run argfunction if $* returns true
// returns the value returned by the argfunction
int if_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  else if (!argv.var_exists("IF_TEST")) {
    argv.global_var("IF_TEST", "false"); 
    return if_core(argv, true);}
  else {
    argv.unset_var("IF_TEST");
    throw If_before_else_t();}}

// run argfunction if IF_TEST is false and $* returns true
// returns the value returned by the argfunction
int else_if_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  else if (!argv.var_exists("IF_TEST")) throw Else_without_if_t();
  else if (argv.get_var("IF_TEST") == "true") return dollar_question;
  else if (argv.get_var("IF_TEST") == "false") return if_core(argv, true);
  else throw Else_without_if_t();}

// run argfunction if IF_TEST is false and $* returns false
// returns the value returned by the argfunction
int else_if_not_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  else if (!argv.var_exists("IF_TEST")) throw Else_without_if_t();
  else if (argv.get_var("IF_TEST") == "true") return dollar_question;
  else if (argv.get_var("IF_TEST") == "false") return if_core(argv, false);
  else throw Else_without_if_t();}

// run argfunction if IF_TEST is false 
// returns the value returned by the argfunction
int else_bi(const Argv_t& argv) {
  int ret;
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  else if (!argv.var_exists("IF_TEST")) {
    argv.unset_var("IF_TEST");
    throw Else_without_if_t();}
  else if (argv.get_var("IF_TEST") == "true") ret = dollar_question;
  else if (argv.get_var("IF_TEST") == "false") 
    if (argv.argfunction()) {
      argv.unset_var("IF_TEST");
      Argv_t mapped_argv;
      mapped_argv.push_back("rwsh.mapped_argfunction");
      mapped_argv.output = argv.output.child_stream();
      (*argv.argfunction())(mapped_argv);
      if (argv.var_exists("IF_TEST")) {
        argv.unset_var("IF_TEST");
        throw Bad_if_nest_t();}
      else ret = dollar_question;}
    else ret = 0;
  else {
    argv.unset_var("IF_TEST");
    throw Else_without_if_t();}
  argv.unset_var("IF_TEST");
  return ret;}

// import the external environment into the variable map, overwriting variables
// that already exist
int importenv_overwrite_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos) 
      if (argv.var_exists(src.substr(0, div)))
        argv.set_var(src.substr(0, div), src.substr(div+1));
      else argv.global_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// import the external environment into the variable map, preserving variables
// that already exist
int importenv_preserve_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos && !argv.var_exists(src.substr(0, div))) 
      argv.global_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// returns one if the input stream is not the default_stream
int is_default_input_bi(const Argv_t& argv) {
  if(argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  return !argv.input.is_default();}

// returns one if the output stream is not the default_stream
int is_default_output_bi(const Argv_t& argv) {
  if(argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  return !argv.output.is_default();}

// returns one if the error stream is not the default_stream
int is_default_error_bi(const Argv_t& argv) {
  if(argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  return !argv.error.is_default();}

// list the files specified by the arguments if they exist
int ls_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  struct stat sb;
  int ret = 1;
  for (Argv_t::const_iterator i=argv.begin(); i != argv.end(); ++i) 
    if (!stat(i->c_str(), &sb)) {
      argv.output <<*i <<"\n";
      ret = 0;}
  argv.output.flush();
  return ret;}

// ignore arguments, and then do nothing
int nop_bi(const Argv_t& argv) {return dollar_question;}

// return the value given by the argument
int return_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  try {return my_strtoi(argv[1]);}
  catch (E_generic_t) {throw Not_a_number_t(argv[1]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[1]);}
  catch (E_range_t) {throw Input_range_t(argv[1]);}}

// modify variable $1 as a selection according to $2
int selection_set_bi(const Argv_t& argv) {
  if (argv.size() < 3) throw Argument_count_t(argv.size(), 3);
  std::list<Entry_pattern_t> focus;
  try {str_to_entry_pattern_list(argv.get_var(argv[1]), focus);}
  catch (Undefined_variable_t error) {return -1;}
  std::string change = *(argv.begin()+2);
  for (Argv_t::const_iterator i = argv.begin()+3; i != argv.end(); ++i) 
    change += ' ' + *i;
  str_to_entry_pattern_list(change, focus);
  argv.set_var(argv[1], entry_pattern_list_to_str(focus.begin(), focus.end()));
  return 0;}

// set variable $1 to $*2
// returns 1 if the variable does not exist
int set_bi(const Argv_t& argv) {
  if (argv.size() < 3) throw Argument_count_t(argv.size(), 3);
  if (isargvar(argv[1]) || argv[1] == "IF_TEST") return 2;
  std::string dest("");
  for (Argv_t::const_iterator i = argv.begin()+2; i != argv.end()-1; ++i) 
    dest += *i + ' ';
  dest += argv.back();
  try {return argv.set_var(argv[1], dest);}
  catch (Undefined_variable_t) {return -1;}}

// run the first argument as if it was a script, passing additional arguments
// to that script
// returns last return value from script, -1 if empty
int source_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  struct stat sb;
  if (stat(argv[1].c_str(), &sb)) {throw File_open_failure_t(argv[1]);}
  if (!(sb.st_mode & S_IXUSR)) {throw Not_executable_t(argv[1]);}
  std::ifstream src(argv[1].c_str(), std::ios_base::in);
  Argv_t script_arg(argv.begin()+1, argv.end(), 0, argv.input,
                    argv.output.child_stream(), argv.error);
  Command_stream_t command_stream(src, false);
  Arg_script_t script("", 0);
  int ret = -1;
  while (command_stream && !Executable_t::unwind_stack()) {
    Argv_t command;
    try {
      if (!(command_stream >> script)) break;
      command = script.interpret(script_arg);}
    catch (Argv_t exception) {command = exception;}
    ret = executable_map.run(command);}
  return ret;}

// run the argument function once with each command in the specified function
// invocation
int stepwise_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  if (!argv.argfunction()) throw Missing_argfunction_t();
  Argv_t lookup(argv.begin()+1, argv.end(), 0,
                argv.input, argv.output.child_stream(), argv.error);
  Executable_t* e = executable_map.find(lookup);
  if (!e) return 1;  // executable not found
  Function_t* f = dynamic_cast<Function_t*>(e);
  if (!f) return 2; // the named executable is not a function
  if (f->increment_nesting(lookup)) return dollar_question;
  int ret = -1;
  for (Function_t::const_iterator i = f->script.begin(); 
       i != f->script.end(); ++i) {
    Argv_t body;
    try {body = i->interpret(lookup);}
    catch (Failed_substitution_t error) {break;}
    catch (Undefined_variable_t error) {break;}
    body.push_front("rwsh.mapped_argfunction");
    ret  = (*argv.argfunction())(body);
    if (Executable_t::unwind_stack()) {
      ret = -1;
      break;}}
  if (f->decrement_nesting(lookup)) ret = dollar_question;
  return ret;} // last return value from argfunction

// run the argfunction and store its output in the variable $1
// returns the last return from the argfunction
int store_output_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  if (!argv.argfunction()) throw Missing_argfunction_t();
  if (isargvar(argv[1]) || argv[1] == "IF_TEST") return 2;
  Argv_t mapped_argv;
  mapped_argv.push_back("rwsh.mapped_argfunction");
  Substitution_stream_t text;
  mapped_argv.output = text.child_stream();
  int ret = (*argv.argfunction())(mapped_argv);
  if (Executable_t::unwind_stack()) return -1;
  if (ret) return ret;
  try {argv.set_var(argv[1], text.value());}
  catch (Undefined_variable_t) {return -1;}
  return 0;}

// return true if the two strings are the same
int test_string_equal_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  else return argv[1] != argv[2];} // C++ and shell have inverted logic

// return true if two strings convert to a doubles and first is greater
int test_greater_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  double lhs, rhs;
  try {lhs = my_strtod(argv[1]);}
  catch (E_generic_t) {throw Not_a_number_t(argv[1]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[1]);}
  catch (E_range_t) {throw Input_range_t(argv[1]);}
  try {rhs = my_strtod(argv[2]);}
  catch (E_generic_t) {throw Not_a_number_t(argv[2]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[2]);}
  catch (E_range_t) {throw Input_range_t(argv[2]);}
  return lhs <= rhs;} // C++ and shell have inverted logic

// return true if the string converts to a number
int test_is_number_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  try {
    (void) my_strtod(argv[1]);
    return 0;}
  catch (E_generic_t) {return 1;}
  catch (E_nan_t) {return 1;}
  catch (E_range_t) {return 2;}}

// return true if two strings convert to a doubles and first is less
int test_less_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  double lhs, rhs;
  try {lhs = my_strtod(argv[1]);}
  catch (E_generic_t) {throw Not_a_number_t(argv[1]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[1]);}
  catch (E_range_t) {throw Input_range_t(argv[1]);}
  try {rhs = my_strtod(argv[2]);}
  catch (E_generic_t) {throw Not_a_number_t(argv[2]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[2]);}
  catch (E_range_t) {throw Input_range_t(argv[2]);}
  return lhs >= rhs;} // C++ and shell have inverted logic

// return true if the string is not empty
int test_not_empty_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  else return !argv[1].length();} // C++ and shell have inverted logic

// return true if the two strings are different 
int test_string_unequal_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  else return argv[1] == argv[2];} // C++ and shell have inverted logic

// returns true if the two strings 
int test_number_equal_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  double lhs, rhs;
  try {lhs = my_strtod(argv[1]);}
  catch (E_generic_t) {throw Not_a_number_t(argv[1]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[1]);}
  catch (E_range_t) {throw Input_range_t(argv[1]);}
  try {rhs = my_strtod(argv[2]);}
  catch (E_generic_t) {throw Not_a_number_t(argv[2]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[2]);}
  catch (E_range_t) {throw Input_range_t(argv[2]);}
  return lhs != rhs;} // C++ and shell have inverted logic

// removes the given variable from the variable map. you could be really 
// pedantic and throw an rwsh.undefined_variable if it doesn't exist, but the
// fact is that the requested state (one where this variable isn't set) is
// already the case, so it's hard to say what you're protecting people from.
int unset_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  else return argv.unset_var(argv[1]);}

// sleep for the specified number of microseconds
int usleep_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  try {
    int delay = my_strtoi(argv[1], 0, INT_MAX);
    return usleep(delay);}
  catch (E_generic_t) {throw Not_a_number_t(argv[1]);}
  catch (E_nan_t) {throw Not_a_number_t(argv[1]);}
  catch (E_range_t) {throw Input_range_t(argv[1]);}}
  
int var_add_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  try {
    const std::string& var_str = argv.get_var(argv[1]);
    double var_term;
    try {var_term = my_strtod(var_str);}
    catch (E_generic_t) {throw Not_a_number_t(var_str);}
    catch (E_nan_t) {throw Not_a_number_t(var_str);}
    catch (E_range_t) {throw Input_range_t(var_str);}
    double const_term;
    try {const_term = my_strtod(argv[2]);}
    catch (E_generic_t) {throw Not_a_number_t(argv[2]);}
    catch (E_nan_t) {throw Not_a_number_t(argv[2]);}
    catch (E_range_t) {throw Input_range_t(argv[2]);}
    double sum = var_term + const_term;
    if (sum == 1e309 || sum == -1e309) throw Result_range_t(var_str, argv[2]);
    std::ostringstream tmp; 
    tmp <<sum;
    argv.set_var(argv[1], tmp.str());
    return 0;}
  catch (Undefined_variable_t error) {return -1;}}

int var_subtract_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  try {
    const std::string& var_str = argv.get_var(argv[1]);
    double var_term;
    try {var_term = my_strtod(var_str);}
    catch (E_generic_t) {throw Not_a_number_t(var_str);}
    catch (E_nan_t) {throw Not_a_number_t(var_str);}
    catch (E_range_t) {throw Input_range_t(var_str);}
    double const_term;
    try {const_term = my_strtod(argv[2]);}
    catch (E_generic_t) {throw Not_a_number_t(argv[2]);}
    catch (E_nan_t) {throw Not_a_number_t(argv[2]);}
    catch (E_range_t) {throw Input_range_t(argv[2]);}
    double difference = var_term - const_term;
    if (difference == 1e309 || difference == -1e309)
      throw Result_range_t(var_str, argv[2]);
    std::ostringstream tmp; 
    tmp <<difference;
    argv.set_var(argv[1], tmp.str());
    return 0;}
  catch (Undefined_variable_t error) {return -1;}}

int var_divide_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  try {
    const std::string& var_str = argv.get_var(argv[1]);
    double var_term;
    try {var_term = my_strtod(var_str);}
    catch (E_generic_t) {throw Not_a_number_t(var_str);}
    catch (E_nan_t) {throw Not_a_number_t(var_str);}
    catch (E_range_t) {throw Input_range_t(var_str);}
    double const_term;
    try {const_term = my_strtod(argv[2]);}
    catch (E_generic_t) {throw Not_a_number_t(argv[2]);}
    catch (E_nan_t) {throw Not_a_number_t(argv[2]);}
    catch (E_range_t) {throw Input_range_t(argv[2]);}
    if (const_term == 0) throw Divide_by_zero_t(var_str);
    double quotient = var_term / const_term;
    if (quotient == 0 && var_term != 0) throw Result_range_t(var_str, argv[2]);
    std::ostringstream tmp; 
    tmp <<quotient;
    argv.set_var(argv[1], tmp.str());
    return 0;}
  catch (Undefined_variable_t error) {return -1;}}

int var_exists_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  else return !argv.var_exists(argv[1]);}

static const std::string version_str("0.2.1+");

// write to standard output the version of rwsh
int version_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  argv.output <<version_str;
  return 0;}

// write to standard output a list of the version with which this shell is 
// compatible
int version_available_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  argv.output <<version_str;
  argv.output.flush();
  return 0;}

// return true if the given version string is compatible with the version
// of this shell
int version_compatible_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 0);
  else if (argv[1] == version_str) return 0;
  else return 1;}

// prints the total amount of time the shell has not been waiting for user input
int waiting_for_binary_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  argv.output <<rwsh_clock.waiting_for_binary();
  argv.output.flush();
  return 0;}

// prints the total amount of time that has passed and the shell has not been
// waiting for other processes or the user
int waiting_for_shell_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 1);
  argv.output <<rwsh_clock.waiting_for_shell();
  argv.output.flush();
  return 0;}

// prints the total amount of time the shell has been waiting for user input
int waiting_for_user_bi(const Argv_t& argv) {
  if (argv.size() != 1) throw Argument_count_t(argv.size(), 0);
  argv.output <<rwsh_clock.waiting_for_user();
  argv.output.flush();
  return 0;}

// print the string corresponding to the executable in the executable map with
// key $1
int which_executable_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable_t* focus = executable_map.find(lookup);
  if (focus) {
    argv.output <<focus->str();
    argv.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int which_execution_count_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable_t* focus = executable_map.find(lookup);
  if (focus) {
    argv.output <<focus->execution_count();
    argv.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int which_last_execution_time_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable_t* focus = executable_map.find(lookup);
  if (focus) {
    struct timeval val = focus->last_execution_time();
    argv.output <<val;
    argv.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// print the number of times that the executable in the executable map with
// key $1 has been run
int which_total_execution_time_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable_t* focus = executable_map.find(lookup);
  if (focus) {
    struct timeval val = focus->total_execution_time();
    argv.output <<val;
    argv.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// find the binary in $2 with filename $1
int which_path_bi(const Argv_t& argv) {
  if (argv.size() != 3) throw Argument_count_t(argv.size(), 3);
  std::vector<std::string> path;
  tokenize_strict(argv[2], std::back_inserter(path), 
                  std::bind2nd(std::equal_to<char>(), ':'));
  for (std::vector<std::string>::iterator i = path.begin(); i != path.end();
       ++i) {
    std::string test = *i + '/' + argv[1];
    struct stat sb;
    if (!stat(test.c_str(), &sb)) {
      argv.output <<test;
      return 0;}}
  return 1;} // executable does not exist

// prints the last return value of the executable with named $1
int which_return_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  Argv_t lookup(argv.begin()+1, argv.end(), 0,
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.mapped_argfunction" || 
            lookup[0] == "rwsh.argfunction") 
    return 2; // return values not stored for argfunctions
  Executable_t* focus = executable_map.find(lookup);
  if (focus) {
    argv.output <<focus->last_ret();
    argv.output.flush();
    return 0;}
  else return 1;} // executable does not exist

// return true if ther is an executable in the executable map with key $1
int which_test_bi(const Argv_t& argv) {
  if (argv.size() != 2) throw Argument_count_t(argv.size(), 2);
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 
                default_input, default_output, default_error);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  return !executable_map.find(lookup);}

// for each time that the arguments return true, run the argfunction
// returns the last return from the argfunction
int while_bi(const Argv_t& argv) {
  if (argv.size() < 2) throw Argument_count_t(argv.size(), 2);
  int ret = -1;
  Argv_t lookup(argv.begin()+1, argv.end(), 0,
                argv.input, argv.output.child_stream(), argv.error);
  while (!executable_map.run(lookup)) {
    if (Executable_t::unwind_stack()) return -1;
    if (argv.argfunction()) {
      Argv_t mapped_argv;
      mapped_argv.push_back("rwsh.mapped_argfunction");
      mapped_argv.output = argv.output.child_stream();
      ret = (*argv.argfunction())(mapped_argv);
      if (Executable_t::unwind_stack()) return -1;}
    else ret = 0;}
  return ret;}

