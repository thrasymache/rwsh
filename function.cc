// The definition of the Function_t class which can be used to change the 
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2006 Samuel Newbold

#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "tokenize.cc"
#include "variable_map.h"

Function_t::Function_t(const std::string& name_i, const std::string& src) :
  name_v(name_i) {
  std::vector<std::string> commands;
  tokenize_same_brace_level(src, std::back_inserter(commands), 
                            std::bind2nd(std::equal_to<char>(), ';'));
  for (std::vector<std::string>::const_iterator i = commands.begin();
       i != commands.end(); ++i) {
    script.push_back(Arg_script_t(Argv_t(*i)));
    if (commands.size() != 1 && script.back().is_argfunction())
      std::cerr <<"rwsh.argfunction cannot occur as one of several "
                  "commands\n";}}
  
// generate a new function by unescaping argument functions and replacing
// unescaped_argfunction with the argument function in argv
Function_t* Function_t::interpret(const Argv_t& argv) const {
  if (script[0].is_argfunction()) return argv.argfunction()->copy_pointer();
  else {
    Function_t* result = new Function_t(name());
    for (std::vector<Arg_script_t>::const_iterator i = script.begin();
         i != script.end(); ++i) {
      result->script.push_back(Arg_script_t(*i, argv));}
    return result;}}
  
// run the given function
int Function_t::operator() (const Argv_t& src_argv) {
  if (increment_nesting(src_argv)) return dollar_question;
  int ret;
  for (std::vector<Arg_script_t>::const_iterator i = script.begin();
       i != script.end(); ++i) {
    Argv_t dest_argv = i->interpret(src_argv);
    ret = (executable_map[dest_argv])(dest_argv);
    if (unwind_stack()) break;}
  last_return = ret;
  if (decrement_nesting(src_argv)) ret = dollar_question;
  return ret;}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function_t::str() const {
    std::string body;
    for (std::vector<Arg_script_t>::const_iterator i = script.begin();
         i != script.end()-1; ++i) body += i->str() + "; ";
    body += script.back().str();
    if (name() == "rwsh.argfunction") return "{" + body + "}";
    else return "%function " + name_v + " {" + body + "}";}

