// The functions that implement each of the builtin executables
//
// Copyright (C) 2006 Samuel Newbold

#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <map>
#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
extern char** environ;

#include "argv.h"
#include "arg_script.h"
#include "builtin.h"
#include "command_stream.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "rwsh_stream.h"
#include "read_dir.cc"
#include "selection.h"
#include "tokenize.cc"
#include "util.h"
#include "variable_map.h"

// add the given string to the end of ERRNO, creating it if necessary
int append_to_errno_bi(const Argv_t& argv) {
  if (argv.size() < 1) return 1;
  std::string dest("");
  for (Argv_t::const_iterator i = argv.begin()+1; i != argv.end()-1; ++i) 
    dest += *i + ' ';
  dest += argv.back();
  argv.append_to_errno(dest);
  return 0;}

// change the current directory to the one given
// returns the error returned from chdir
int cd_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  errno = 0;
  int ret = chdir(argv[1].c_str());
  if (!ret) argv.set_var("CWD", argv[1]);
  else if (errno == ENOENT) argv.append_to_errno("NOENT");
  else if (errno == ENOTDIR) argv.append_to_errno("NOTDIR"); 
  else argv.append_to_errno("CD_ERROR"); 
  errno = 0;
  return ret;}

// echo arguments to standard output
int echo_bi(const Argv_t& argv) {
  if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  for (Argv_t::const_iterator i = argv.begin()+1; i != argv.end()-1; ++i)
    *argv.myout() <<*i <<" ";
  *argv.myout() <<argv.back();
  argv.myout()->flush();
  return 0;}

// unset ERRNO while running argfunction, then merge errors
int error_unit_bi(const Argv_t& argv) {
  if (!argv.argfunction()) {argv.append_to_errno("ARGFUNCTION"); return -1;}
  Argv_t inner_args(default_stream_p);
  inner_args.push_back("rwsh.mapped_argfunction");
  copy(argv.begin()+1, argv.end(), std::back_inserter(inner_args));
  std::string saved_errno;
  bool previous_error;
  if (argv.var_exists("ERRNO")) {
    previous_error = true;
    saved_errno = argv.get_var("ERRNO");
    argv.unset_var("ERRNO");}
  else previous_error = false;
  (*argv.argfunction())(inner_args);
  if (previous_error)
    if (argv.var_exists("ERRNO"))
      argv.set_var("ERRNO", saved_errno + " " + argv.get_var("ERRNO"));
    else argv.global_var("ERRNO", saved_errno);
  return dollar_question;}

// exit the shell
int exit_bi(const Argv_t& argv) {
  if (argv.size() != 1) {argv.append_to_errno("ARGS"); return -1;}
  Variable_map_t::exit_requested = true;
  return 0;}

// run the argfunction for each argument, passing that value as the argument
// returns the value returned by the argfunction
int for_bi(const Argv_t& argv) {
  if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  int ret = -1;
  Argv_t body(default_stream_p);
  body.push_back("rwsh.mapped_argfunction");
  body.push_back("");
  for (Argv_t::const_iterator i = ++argv.begin(); i != argv.end(); ++i) {
    if (argv.argfunction()) {
      body[1] = *i;
      ret  = (*argv.argfunction())(body);
      if (Executable_t::unwind_stack() || argv.var_exists("ERRNO")) 
        return -1;}
    else ret = 0;}
  return ret;}

// add argfunction to executable map with name $1
int function_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  else if (is_binary_name(argv[1])) return 1;
  else if (is_builtin_name(argv[1])) return 2;
  else if (is_argfunction_name(argv[1])) return 3;
  else if (!argv.argfunction()) {
    return 4 * !executable_map.erase(*(argv.begin()+1));}
  else {
    executable_map.set(new Function_t(argv[1], argv.argfunction()));
    return 0;}}

int global_bi(const Argv_t& argv) {
  if (argv.size() != 3) {argv.append_to_errno("ARGS"); return -1;}
  else return argv.global_var(argv[1], argv[2]);}

static int if_core(const Argv_t& argv, bool logic) {
  Argv_t lookup(argv.begin()+1, argv.end(), 0, argv.myout());
  if (logic == !executable_map.run(lookup)) {
    if (Executable_t::unwind_stack()) return -1;
    argv.unset_var("IF_TEST");
    int ret;
    if (argv.argfunction()) {
      Argv_t mapped_argv(argv.myout());
      mapped_argv.push_back("rwsh.mapped_argfunction");
      ret = (*argv.argfunction())(mapped_argv);}
    else ret = 0;
    if (argv.var_exists("IF_TEST")) {
      argv.append_to_errno("BAD_IF_NEST");
      argv.unset_var("IF_TEST");
      ret = -1;}
    else argv.global_var("IF_TEST", "true");
    return ret;}
  else return 0;}

// run argfunction if $* returns true
// returns the value returned by the argfunction
int if_bi(const Argv_t& argv) {
  if (argv.var_exists("ERRNO")) return dollar_question;
  else if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  else if (!argv.var_exists("IF_TEST")) {
    argv.global_var("IF_TEST", "false"); 
    return if_core(argv, true);}
  else {
    argv.append_to_errno("IF_BEFORE_ELSE");
    argv.unset_var("IF_TEST");
    return -1;}}

// run argfunction if ERRNO is set
// returns the (second?) return value from argfunction
int if_errno_bi(const Argv_t& argv) {
  if (argv.size() != 1) argv.append_to_errno("ARGS");
  if (argv.var_exists("ERRNO") && argv.argfunction()) {
    Argv_t mapped_argv(argv.myout());
    mapped_argv.push_back("rwsh.mapped_argfunction");
    return (*argv.argfunction())(mapped_argv);}
  else return 0;}

// run argfunction if IF_TEST is false and $* returns true
// returns the value returned by the argfunction
int else_if_bi(const Argv_t& argv) {
  if (argv.var_exists("ERRNO")) return dollar_question;
  else if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  else if (!argv.var_exists("IF_TEST")) {
    argv.append_to_errno("ELSE_WITHOUT_IF"); return -1;}
  else if (argv.get_var("IF_TEST") == "true") return dollar_question;
  else if (argv.get_var("IF_TEST") == "false") return if_core(argv, true);
  else {argv.append_to_errno("ELSE_WITHOUT_IF"); return -1;}}

// run argfunction if IF_TEST is false and $* returns false
// returns the value returned by the argfunction
int else_if_not_bi(const Argv_t& argv) {
  if (argv.var_exists("ERRNO")) return dollar_question;
  else if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  else if (!argv.var_exists("IF_TEST")) {
    argv.append_to_errno("ELSE_WITHOUT_IF"); return -1;}
  else if (argv.get_var("IF_TEST") == "true") return dollar_question;
  else if (argv.get_var("IF_TEST") == "false") return if_core(argv, false);
  else {argv.append_to_errno("ELSE_WITHOUT_IF"); return -1;}}

// run argfunction if IF_TEST is false 
// returns the value returned by the argfunction
int else_bi(const Argv_t& argv) {
  int ret;
  if (argv.var_exists("ERRNO")) ret = dollar_question;
  else if (argv.size() != 1) {argv.append_to_errno("ARGS"); ret = -1;}
  else if (!argv.var_exists("IF_TEST")) {
    argv.append_to_errno("ELSE_WITHOUT_IF"); ret = -1;}
  else if (argv.get_var("IF_TEST") == "true") ret = dollar_question;
  else if (argv.get_var("IF_TEST") == "false") 
    if (argv.argfunction()) {
      argv.unset_var("IF_TEST");
      Argv_t mapped_argv(argv.myout());
      mapped_argv.push_back("rwsh.mapped_argfunction");
      (*argv.argfunction())(mapped_argv);
      if (argv.var_exists("IF_TEST")) {
        argv.append_to_errno("BAD_IF_NEST"); ret = -1;}
      else ret = dollar_question;}
    else ret = 0;
  else {argv.append_to_errno("ELSE_WITHOUT_IF"); ret = -1;}
  argv.unset_var("IF_TEST");
  return ret;}

// import the external environment into the variable map, overwriting variables
// that already exist
int importenv_overwrite_bi(const Argv_t& argv) {
  if (argv.size() != 1) {argv.append_to_errno("ARGS"); return -1;}
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
  if (argv.size() != 1) {argv.append_to_errno("ARGS"); return -1;}
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos && !argv.var_exists(src.substr(0, div))) 
      argv.global_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// list the files specified by the arguments if they exist
int ls_bi(const Argv_t& argv) {
  if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  struct stat sb;
  for (Argv_t::const_iterator i=argv.begin(); i != argv.end(); ++i) 
    if (!stat(i->c_str(), &sb)) *argv.myout() <<*i <<"\n";
  argv.myout()->flush();
  return 0;}

// write a newline to the standard output
int newline_bi(const Argv_t& argv) {
  if (argv.size() != 1) {argv.append_to_errno("ARGS"); return -1;}
  else {*argv.myout() <<"\n"; argv.myout()->flush(); return 0;}}

// ignore arguments, and then do nothing
int nop_bi(const Argv_t& argv) {return dollar_question;}

// builtin function for printing the variable map
int printenv_bi(const Argv_t& argv) {
  extern Variable_map_t* vars;
  vars->get("?");
  if (argv.size() < 2) {
    for (Variable_map_t::const_iterator i = vars->begin(); 
         i != vars->end(); ++i)
      *argv.myout() <<i->first <<"=" <<i->second <<"\n";}
  else {
    *argv.myout() <<(*vars)[argv[1]] <<"\n";}
  argv.myout()->flush();
  return 0;}

// return the value given by the argument
int return_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  try {return my_strtoi(argv[1]);}
  catch (E_generic_t) {argv.append_to_errno("RETURN_ERROR"); return -1;}
  catch (E_nan_t) {argv.append_to_errno("NAN"); return -1;}
  catch (E_range_t) {argv.append_to_errno("RANGE"); return -1;}}

// modify variable $1 as a selection according to $2
int selection_set_bi(const Argv_t& argv) {
  if (argv.size() < 3) {argv.append_to_errno("ARGS"); return -1;}
  std::vector<Entry_pattern_t> focus;
  str_to_entry_pattern_vector(argv.get_var(argv[1]), focus);
  for (Argv_t::const_iterator i = argv.begin()+2; i != argv.end(); ++i) 
    str_to_entry_pattern_vector(*i, focus);
  argv.set_var(argv[1], entry_pattern_vector_to_str(focus));
  return 0;}

// set variable $1 to $*2
// returns 1 if the variable does not exist
int set_bi(const Argv_t& argv) {
  if (argv.size() < 3) {argv.append_to_errno("ARGS"); return -1;}
  if (isargvar(argv[1]) || argv[1] == "IF_TEST") return 2;
  std::string dest("");
  for (Argv_t::const_iterator i = argv.begin()+2; i != argv.end()-1; ++i) 
    dest += *i + ' ';
  dest += argv.back();
  return argv.set_var(argv[1], dest);}

// run the first argument as if it was a script, passing additional arguments
// to that script
// returns last return value from script, -1 if empty
int source_bi(const Argv_t& argv) {
  if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  struct stat sb;
  if (stat(argv[1].c_str(), &sb)) {argv.append_to_errno("NOENT"); return -1;}
  if (!(sb.st_mode & S_IXUSR)) {argv.append_to_errno("NOEXEC"); return -1;}
  std::ifstream src(argv[1].c_str(), std::ios_base::in);
  Argv_t script_arg(argv);
  Command_stream_t command_stream(src);
  Arg_script_t script("");
  int ret = -1;
  while (command_stream && !Executable_t::unwind_stack()) {
    Argv_t command(0);
    try {
      if (!(command_stream >> script)) break;
      command = script.interpret(script.argv());}
    catch (Argv_t exception) {command = exception;}
    ret = executable_map.run(command);}
  return ret;}

// run the argument function once with each command in the specified function
// invocation
int stepwise_bi(const Argv_t& argv) {
  if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  if (!argv.argfunction()) {argv.append_to_errno("ARGFUNCTION"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0, argv.myout());
  Executable_t* e = executable_map.find(lookup);
  if (!e) return 1;  // executable not found
  Function_t* f = dynamic_cast<Function_t*>(e);
  if (!f) return 2; // the named executable is not a function
  if (f->increment_nesting(lookup)) return dollar_question;
  int ret = -1;
  for (Function_t::const_iterator i = f->script.begin(); 
       i != f->script.end(); ++i) {
    Argv_t body = i->interpret(lookup);
    body.push_front("rwsh.mapped_argfunction");
    ret  = (*argv.argfunction())(body);
    if (Executable_t::unwind_stack() || argv.var_exists("ERRNO")) {
      ret = -1;
      break;}}
  if (f->decrement_nesting(lookup)) ret = dollar_question;
  return ret;} // last return value from argfunction

// return true if the two strings are the same
int test_equal_bi(const Argv_t& argv) {
  if (argv.size() != 3) {argv.append_to_errno("ARGS"); return -1;}
  else return argv[1] != argv[2];}

// return true if the string is not empty
int test_not_empty_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  else return !argv[1].length();}

// return true if the two strings are different 
int test_not_equal_bi(const Argv_t& argv) {
  if (argv.size() != 3) {argv.append_to_errno("ARGS"); return -1;}
  else return argv[1] == argv[2];}

int unset_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  else return argv.unset_var(argv[1]);}

int var_add_bi(const Argv_t& argv) {
  if (argv.size() != 3) {argv.append_to_errno("ARGS"); return -1;}
  const std::string& var_str = argv.get_var(argv[1]);
  if (var_str == "") return 1; // variable does not exist
  int var_term;
  try {var_term = my_strtoi(var_str);}
  catch (E_generic_t) {argv.append_to_errno("VAR_ADD_ERROR"); return -1;}
  catch (E_nan_t) {argv.append_to_errno("VAR_NAN"); return -1;}
  catch (E_range_t) {argv.append_to_errno("VAR_RANGE"); return -1;}
  int const_term;
  try {const_term = my_strtoi(argv[2]);}
  catch (E_generic_t) {argv.append_to_errno("VAR_ADD_ERROR"); return -1;}
  catch (E_nan_t) {argv.append_to_errno("CONST_NAN"); return -1;}
  catch (E_range_t) {argv.append_to_errno("CONST_RANGE"); return -1;}
  int sum = var_term + const_term;
  int var_negative = var_term < 0;
  if (var_negative == (const_term < 0) && var_negative != (sum < 0)) {
    argv.append_to_errno("SUM_RANGE"); return -1;}
  std::ostringstream tmp; 
  tmp <<sum;
  argv.set_var(argv[1], tmp.str());
  return 0;}

int var_exists_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  else return !argv.var_exists(argv[1]);}

static const std::string version_str("0.2+");

// write to standard output the version of rwsh
int version_bi(const Argv_t& argv) {
  if (argv.size() != 1) {argv.append_to_errno("ARGS"); return -1;}
  *argv.myout() <<version_str;
  return 0;}

// write to standard output a list of the version with which this shell is 
// compatible
int version_available_bi(const Argv_t& argv) {
  if (argv.size() != 1) {argv.append_to_errno("ARGS"); return -1;}
  *argv.myout() <<version_str;
  argv.myout()->flush();
  return 0;}

// return true if the given version string is compatible with the version
// of this shell
int version_compatible_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  else if (argv[1] == version_str) return 0;
  else return 1;}

// print the string corresponding to the executable in the executable map with
// key $1
int which_executable_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 0);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  Executable_t* focus = executable_map.find(lookup);
  if (focus) {
    *argv.myout() <<focus->str();
    argv.myout()->flush();
    return 0;}
  else return 1;} // executable does not exist

// find the binary in $PATH with filename $1
int which_path_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0, 0);
  std::vector<std::string> path;
  tokenize_strict(argv.get_var("PATH"), std::back_inserter(path), 
                  std::bind2nd(std::equal_to<char>(), ':'));
  for (std::vector<std::string>::iterator i = path.begin(); i != path.end();
       ++i) {
    std::string test = *i + '/' + lookup[0];
    struct stat sb;
    if (!stat(test.c_str(), &sb)) {
      *argv.myout() <<test;
      return 0;}}
  return 1;} // executable does not exist

// insert into the executable map a function with name $1 that runs the binary 
// in $PATH with filename $1 with arguments $*2
int autofunction_bi(const Argv_t& argv) {
  if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0, 0);
  std::vector<std::string> path;
  tokenize_strict(argv.get_var("PATH"), std::back_inserter(path), 
                  std::bind2nd(std::equal_to<char>(), ':'));
  for (std::vector<std::string>::iterator i = path.begin(); i != path.end();
       ++i) {
    std::string test = *i + '/' + lookup[0];
    struct stat sb;
    if (!stat(test.c_str(), &sb)) {
      std::string script;
      for (Argv_t::const_iterator j = lookup.begin()+1; j != lookup.end(); 
           ++j) script += " " + *j;
      executable_map.set(new Function_t(lookup[0], test + script));
      return 0;}}
  return 1;} // executable does not exist

// prints the last return value of the executable with named $1
int which_return_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0, 0);
  if (lookup[0] == "rwsh.mapped_argfunction" || 
            lookup[0] == "rwsh.argfunction") 
    return 2; // return values not stored for argfunctions
  Executable_t* focus = executable_map.find(lookup);
  if (focus) {
    *argv.myout() <<focus->last_ret();
    argv.myout()->flush();
    return 0;}
  else return 1;} // executable does not exist

// return true if ther is an executable in the executable map with key $1
int which_test_bi(const Argv_t& argv) {
  if (argv.size() != 2) {argv.append_to_errno("ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), argv.argfunction(), 0);
  if (lookup[0] == "rwsh.argfunction") lookup[0] = "rwsh.mapped_argfunction";
  return !executable_map.find(lookup);}

// for each time that the arguments return true, run the argfunction
// returns the last return from the argfunction
int while_bi(const Argv_t& argv) {
  if (argv.size() < 2) {argv.append_to_errno("ARGS"); return -1;}
  int ret = -1;
  Argv_t lookup(argv.begin()+1, argv.end(), 0, argv.myout());
  while (!executable_map.run(lookup)) {
    if (Executable_t::unwind_stack() || argv.var_exists("ERRNO")) return -1;
    if (argv.argfunction()) {
      Argv_t mapped_argv(argv.myout());
      mapped_argv.push_back("rwsh.mapped_argfunction");
      ret = (*argv.argfunction())(mapped_argv);
      if (Executable_t::unwind_stack() || argv.var_exists("ERRNO"))
        return -1;}
    else ret = 0;}
  return ret;}

