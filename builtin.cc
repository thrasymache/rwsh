// The functions that implement each of the builtin executables
//
// Copyright (C) 2006 Samuel Newbold

#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stdio.h>
#include <string>
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
#include "read_dir.cc"
#include "selection.h"
#include "tokenize.cc"
#include "variable_map.h"

// change the current directory to the one given
int cd_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  errno = 0;
  int ret = chdir(argv[1].c_str());
  if (!ret) set_var("CWD", argv[1]);
  else if (errno == ENOENT) set_var("ERRNO", "NOENT");
  else if (errno == ENOTDIR) set_var("ERRNO", "NOTDIR"); 
  else set_var("ERRNO", "CD_ERROR"); 
  errno = 0;
  return ret;}

// echo arguments to standard output
int echo_bi(const Argv_t& argv) {
  if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  for (Argv_t::const_iterator i = argv.begin()+1; i != argv.end()-1; ++i)
    std::cout <<*i <<' ';
  std::cout <<argv.back();
  std::cout.flush();
  return 0;}

static int if_core(const Argv_t& argv, bool logic) {
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  if (logic == !executable_map[lookup](lookup)) {
    if (Executable_t::unwind_stack()) return -1;
    set_var("IF_TEST", "");
    int ret;
    if (argv.argfunction())
      ret  = (*argv.argfunction())(Argv_t("rwsh.mapped_argfunction"));
    else ret = 0;
    if (get_var("IF_TEST") != "") {set_var("ERRNO", "BAD_IF_NEST"); ret = -1;}
    set_var("IF_TEST", "true");
    return ret;}
  else return 0;}

// run argfunction if $* returns true
int if_bi(const Argv_t& argv) {
  if (get_var("ERRNO") != "") return dollar_question;
  else if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  else if (get_var("IF_TEST") == "") {
    set_var("IF_TEST", "false"); 
    return if_core(argv, true);}
  else {set_var("ERRNO", "IF_BEFORE_ELSE"); return -1;}}

// run argfunction if ERRNO is set
int if_errno_bi(const Argv_t& argv) {
  int ret = 0;
  if (get_var("ERRNO") != "" && argv.argfunction()) 
    ret = (*argv.argfunction())(Argv_t("rwsh.mapped_argfunction"));
  if (argv.size() != 1) {
    set_var("ERRNO", "ARGS");
    if (argv.argfunction()) 
      ret = (*argv.argfunction())(Argv_t("rwsh.mapped_argfunction"));}
  return ret;}

// run argfunction if IF_TEST is false and $* returns true
int else_if_bi(const Argv_t& argv) {
  if (get_var("ERRNO") != "") return dollar_question;
  else if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  else if (get_var("IF_TEST") == "true") return dollar_question;
  else if (get_var("IF_TEST") == "false") return if_core(argv, true);
  else {set_var("ERRNO", "ELSE_WITHOUT_IF"); return -1;}}

// run argfunction if IF_TEST is false and $* returns false
int else_if_not_bi(const Argv_t& argv) {
  if (get_var("ERRNO") != "") return dollar_question;
  else if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  else if (get_var("IF_TEST") == "true") return dollar_question;
  else if (get_var("IF_TEST") == "false") return if_core(argv, false);
  else {set_var("ERRNO", "ELSE_WITHOUT_IF"); return -1;}}

// run argfunction if IF_TEST is false 
int else_bi(const Argv_t& argv) {
  int ret;
  if (get_var("ERRNO") != "") ret = dollar_question;
  else if (argv.size() != 1) {set_var("ERRNO", "ARGS"); ret = -1;}
  else if (get_var("IF_TEST") == "true") ret = dollar_question;
  else if (get_var("IF_TEST") == "false") 
    if (argv.argfunction()) {
      set_var("IF_TEST", "");
      (*argv.argfunction())(Argv_t("rwsh.mapped_argfunction"));
      if (get_var("IF_TEST") != "") {set_var("ERRNO", "BAD_IF_NEST"); ret = -1;}
      else ret = dollar_question;}
    else ret = 0;
  else {set_var("ERRNO", "ELSE_WITHOUT_IF"); ret = -1;}
  set_var("IF_TEST", "");
  return ret;}

// exit the shell
int exit_bi(const Argv_t& argv) {
  if (argv.size() != 1) {set_var("ERRNO", "ARGS"); return -1;}
  exit_requested = true;
  return 0;}

// run the argfunction for each argument, passing that value as the argument
int for_bi(const Argv_t& argv) {
  if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  int ret = -1;
  Argv_t body("rwsh.mapped_argfunction");
  body.push_back("");
  for (Argv_t::const_iterator i = ++argv.begin(); i != argv.end(); ++i) {
    if (argv.argfunction()) {
      body[1] = *i;
      ret  = (*argv.argfunction())(body);
      if (Executable_t::unwind_stack() || get_var("ERRNO") != "") return -1;}
    else ret = 0;}
  return ret;}

// add argfunction to executable map with name $1
int function_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  else if (argv[1][0] == '/') return 1;
  else if (argv[1][0] == '%') return 2;
  else if (is_argfunction_name(argv[1])) return 3;
  else if (!argv.argfunction()) {
    Argv_t lookup(argv.begin()+1, argv.end(), 0);
    return 4 * !executable_map.erase(lookup);}
  else {
    executable_map.set(new Function_t(argv[1], argv.argfunction()));
    return 0;}}

// import the external environment into the variable map, overwriting variables
// that already exist
int importenv_overwrite_bi(const Argv_t& argv) {
  if (argv.size() != 1) {set_var("ERRNO", "ARGS"); return -1;}
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos) 
      set_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// import the external environment into the variable map, preserving variables
// that already exist
int importenv_preserve_bi(const Argv_t& argv) {
  if (argv.size() != 1) {set_var("ERRNO", "ARGS"); return -1;}
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos && get_var(src.substr(0, div)) == "") 
      set_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// list the files specified by the arguments if they exist
int ls_bi(const Argv_t& argv) {
  if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  struct stat sb;
  for (Argv_t::const_iterator i=argv.begin(); i != argv.end(); ++i) 
    if (!stat(i->c_str(), &sb)) std::cout <<*i <<'\n';
  return 0;}

// write a newline to the standard output
int newline_bi(const Argv_t& argv) {
  if (argv.size() != 1) {set_var("ERRNO", "ARGS"); return -1;}
  else {std::cout <<std::endl; return 0;}}

// ignore arguments, and then do nothing
int nop_bi(const Argv_t& argv) {return dollar_question;}

// return the value given by the argument
int return_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  const char* focus = argv[1].c_str();
  char* endptr;
  errno = 0;
  long ret = strtol(focus, &endptr, 10);
  if (!*focus || *endptr) {set_var("ERRNO", "NAN"); return -1;}
  if (errno == ERANGE) set_var("ERRNO", "RANGE");
  if (errno == EINVAL) {set_var("ERRNO", "INVAL"); return -1;}
  else if (ret < INT_MIN) {set_var("ERRNO", "RANGE"); return INT_MIN;}
  else if (ret > INT_MAX) {set_var("ERRNO", "RANGE"); return INT_MAX;}
  else if (errno) {set_var("ERRNO", "RETURN_ERROR"); return -1;}
  else return ret;}

// modify variable $1 as a selection according to $2
int selection_set_bi(const Argv_t& argv) {
  if (argv.size() < 3) {set_var("ERRNO", "ARGS"); return -1;}
  std::string dest(get_var(argv[1]));
  for (Argv_t::const_iterator i = argv.begin()+2; i != argv.end(); ++i) 
    selection_write(*i, dest);
  set_var(argv[1], dest);
  return 0;}

// set variable $1 to $*2
int set_bi(const Argv_t& argv) {
  if (argv.size() < 3) {set_var("ERRNO", "ARGS"); return -1;}
  if (isargvar(argv[1])) return 1;
  std::string dest("");
  if (argv.size() > 2) {
    for (Argv_t::const_iterator i = argv.begin()+2; i != argv.end()-1; ++i) 
      dest += *i + ' ';
    dest += argv.back();}
  set_var(argv[1], dest);
  return 0;}

// run the first argument as if it was a script, passing additional arguments
// to that script
int source_bi(const Argv_t& argv) {
  if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  struct stat sb;
  if (stat(argv[1].c_str(), &sb)) {set_var("ERRNO", "NOENT"); return -1;}
  if (!(sb.st_mode & S_IXUSR)) {set_var("ERRNO", "NOEXEC"); return -1;}
  std::ifstream src(argv[1].c_str(), std::ios_base::in);
  Argv_t script_arg(argv);
  Command_stream_t script(src);
  Argv_t command;
  int ret = -1;
  while (script >> command) {
    try {
      Arg_script_t script(command);
      command = script.interpret(script_arg);}
    catch (Argv_t exception) {command = exception;}
    ret = executable_map[command](command);}
  return ret;}

// run the argument function once with each command in the specified function
// invocation
int stepwise_bi(const Argv_t& argv) {
  if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  if (!argv.argfunction()) {set_var("ERRNO", "ARGFUNCTION"); return -1;}
  // return 1 if the specified command is not a function
  // generate the lookup for the function, look it up, and convince it that it
  //    is executing
  // iterate over your captive function, calling the argfunction
  return 0;}

// return true if the two strings are the same
int test_equal_bi(const Argv_t& argv) {
  if (argv.size() != 3) {set_var("ERRNO", "ARGS"); return -1;}
  else return argv[1] != argv[2];}

// return true if the string is not empty
int test_not_empty_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  else return !argv[1].length();}

// return the string corresponding to the executable in the executable map with
// key $1
int which_executable_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  Executable_map_t::iterator focus = executable_map.find(lookup);
  if (focus != executable_map.end()) {
    std::cout <<focus->second->str() <<std::endl;
    return 0;}
  else if ((lookup[0] == "rwsh.mapped_argfunction" || 
            lookup[0] == "rwsh.argfunction") 
           && argv.argfunction()) {
    std::cout <<argv.argfunction()->str() <<std::endl;
    return 0;}
  else return 1;}

// find the binary in $PATH with filename $1
int which_path_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  std::vector<std::string> path;
  tokenize_strict(get_var("PATH"), std::back_inserter(path), 
                  std::bind2nd(std::equal_to<char>(), ':'));
  for (std::vector<std::string>::iterator i = path.begin(); i != path.end();
       ++i) {
    std::string test = *i + '/' + lookup[0];
    struct stat sb;
    if (!stat(test.c_str(), &sb)) {
      std::cout <<test <<std::endl;
      return 0;}}
  return 1;}

// insert into the executable map a function with name $1 that runs the binary 
// in $PATH with filename $1 with arguments $*2
int autofunction_bi(const Argv_t& argv) {
  if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  std::vector<std::string> path;
  tokenize_strict(get_var("PATH"), std::back_inserter(path), 
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
  return 1;}

// prints the last return value of the executable with named $1
int which_return_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  Executable_map_t::iterator focus = executable_map.find(lookup);
  if (focus != executable_map.end()) {
    std::cout <<focus->second->last_ret() <<std::endl;
    return 0;}
  else if ((lookup[0] == "rwsh.mapped_argfunction" || 
            lookup[0] == "rwsh.argfunction") 
           && argv.argfunction()) 
    return 2;
  else return 1;}

// return true if ther is an executable in the executable map with key $1
int which_test_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  Executable_map_t::iterator focus = executable_map.find(lookup);
  if (focus != executable_map.end()) return 0;
  else if ((lookup[0] == "rwsh.mapped_argfunction" || 
            lookup[0] == "rwsh.argfunction") 
           && argv.argfunction()) 
    return 0;
  else return 1;}

// for each time that the arguments return true, run the argfunction
int while_bi(const Argv_t& argv) {
  if (argv.size() < 2) {set_var("ERRNO", "ARGS"); return -1;}
  int ret = -1;
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  while (!executable_map[lookup](lookup)) {
    if (Executable_t::unwind_stack() || get_var("ERRNO") != "") return -1;
    if (argv.argfunction()) {
      ret  = (*argv.argfunction())(Argv_t("rwsh.mapped_argfunction"));
      if (Executable_t::unwind_stack() || get_var("ERRNO") != "") return -1;}
    else ret = 0;}
  return ret;}

static const std::string version_str("0.2+");

// write to standard output the version of rwsh
int version_bi(const Argv_t& argv) {
  if (argv.size() != 1) {set_var("ERRNO", "ARGS"); return -1;}
  std::cout <<version_str;
  return 0;}

// write to standard output a list of the version with which this shell is 
// compatible
int version_available_bi(const Argv_t& argv) {
  if (argv.size() != 1) {set_var("ERRNO", "ARGS"); return -1;}
  std::cout <<version_str;
  return 0;}

// return true if the given version string is compatible with the version
// of this shell
int version_compatible_bi(const Argv_t& argv) {
  if (argv.size() != 2) {set_var("ERRNO", "ARGS"); return -1;}
  else if (argv[1] == version_str) return 0;
  else return 1;}

