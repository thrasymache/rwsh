// The functions that implement each of the builtin executables
//
// Copyright (C) 2005, 2006 Samuel Newbold

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

// echo arguments to standard output
int echo_bi(const Argv_t& argv) {
  if (argv.size() < 2) return 0;
  for (Argv_t::const_iterator i = argv.begin()+1; i != argv.end()-1; ++i)
    std::cout <<*i <<' ';
  std::cout <<argv.back();
  std::cout.flush();
  return 0;}

// run argfunction if IF_TEST is false and $* returns true
int elif_bi(const Argv_t& argv) {
  if (argv.size() < 2) return 1;
  if (get_var("IF_TEST") == "true");
  else if (get_var("IF_TEST") == "false") {
    Argv_t lookup(argv.begin()+1, argv.end(), 0);
    if (!executable_map[lookup](lookup)) {
      if (Executable_t::excessive_nesting()) return dollar_question;
      set_var("IF_TEST", "");
      (*argv.argfunction())(Argv_t());
      set_var("IF_TEST", "true");}}
  else std::cout <<"syntax error: elif without preceeding if\n";
  return dollar_question;}

// exit the shell
int exit_bi(const Argv_t& argv) {
  exit_requested = true;
  return 0;}

// return failure regardless of arguments
int false_bi(const Argv_t& argv) {return 1;}

// add argfunction to executable map with name $1
int function_bi(const Argv_t& argv) {
  if (argv.size() < 2) return 1;
  else if (argv[1][0] == '/') {
    std::cerr <<"cannot set paths as functions\n"; 
    return 2;}
  else if (argv[1][0] == '%') {
    std::cerr <<"cannot set builtins as functions\n"; 
    return 2;}
  else if (is_argfunction_name(argv[1])) {
    std::cerr <<"cannot set rwsh.argfunction or variants in executable map\n";
    return 2;}
  else if (!argv.argfunction()) {
    Argv_t lookup(argv.begin()+1, argv.end(), 0);
    return !executable_map.erase(lookup);}
  else {
    executable_map.set(new Function_t(argv[1], argv.argfunction()));
    return 0;}}

// import the external environment into the variable map
int importenv_bi(const Argv_t& argv) {
  for (char** i=environ; *i; ++i) {
    std::string src(*i);
    std::string::size_type div = src.find("=");
    if (div != std::string::npos) 
      set_var(src.substr(0, div), src.substr(div+1));}
  return 0;}

// list the files specified by the arguments if they exist
int ls_bi(const Argv_t& argv) {
  if (argv.size() < 2) return 0;
  //std::vector<std::string> dest;
  //if (int ret = read_dir(argv[1], std::back_inserter(dest))) return ret;
  //for (std::vector<std::string>::iterator i=dest.begin(); i != dest.end(); ++i) std::cout <<*i <<'\n';
  struct stat sb;
  for (Argv_t::const_iterator i=argv.begin(); i != argv.end(); ++i) 
    if (!stat(i->c_str(), &sb)) std::cout <<*i <<'\n';
  return 0;}

// write a newline to the standard output
int newline_bi(const Argv_t& argv) {std::cout <<std::endl; return 0;}

// modify variable $1 as a selection according to $2
int selection_set_bi(const Argv_t& argv) {
  if (argv.size() < 3) return 1;
  std::string dest(get_var(argv[1]));
  selection_write(argv[2], dest);
  set_var(argv[1], dest);
  return 0;}

// set variable $1 to $*2
int set_bi(const Argv_t& argv) {
  if (argv.size() < 2) return 1;
  std::string dest("");
  if (argv.size() > 2) {
    for (Argv_t::const_iterator i = argv.begin()+2; i != argv.end()-1; ++i) 
      dest += *i + ' ';
    dest += argv.back();}
  set_var(argv[1], dest);
  return 0;}

// run the specified argument as if it was a script
int source_bi(const Argv_t& argv) {
  std::ifstream src(argv[1].c_str(), std::ios_base::in);
  Executable_map_t::iterator e = 
    executable_map.find(Argv_t("rwsh.before_script"));
  if (e != executable_map.end()) (*e->second)(argv);
  if (Executable_t::excessive_nesting()) return dollar_question;
  Argv_t script_arg(argv.begin()+1, argv.end(), argv.argfunction());
  Command_stream_t script(src);
  Argv_t command;
  int ret = -1;
  while (script >> command) {
    try {
      Arg_script_t script(command);
      command = script.interpret(script_arg);}
    catch (Argv_t exception) {command = exception;}
    ret = executable_map[command](command);}
  if (Executable_t::excessive_nesting()) return dollar_question;
  e = executable_map.find(Argv_t("rwsh.after_script"));
  if (e != executable_map.end()) (*e->second)(argv);
  return ret;}

// return success regardless of arguments
int true_bi(const Argv_t& argv) {return 0;}

// return the string corresponding to the executable in the executable map with
// key $1
int which_bi(const Argv_t& argv) {
  if (argv.size() < 2) return 1;
  Argv_t lookup(argv.begin()+1, argv.end(), 0);
  Executable_map_t::iterator focus = executable_map.find(lookup);
  int ret = 1;
  if (focus != executable_map.end()) {
    std::cout <<focus->second->str() <<std::endl;
    ret = 0;}
  else if ((lookup[0] == "rwsh.mapped_argfunction" || 
            lookup[0] == "rwsh.argfunction") 
           && argv.argfunction()) {
    std::cout <<argv.argfunction()->str() <<std::endl;
    ret = 0;}
  if (ret) std::cout <<"no " <<lookup[0] <<std::endl;
  return ret;}

// insert into the executable map a function with name $1 that runs the binary 
// in $PATH with filename $1 with arguments $*2
int autofunction_bi(const Argv_t& argv) {
  if (argv.size() < 2) return 1;
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

// write to standard output the version of rwsh
int version_bi(const Argv_t& argv) {
  std::cout <<"rwsh v0.1+\n";
  return 0;}

