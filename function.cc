// The definition of the Function_t class which can be used to change the 
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "argv.h"
#include "arg_script.h"
#include "builtin.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "tokenize.cc"
#include "variable_map.h"

int Function_t::global_nesting(0);
bool Function_t::excessive_nesting(false);
bool Function_t::in_excessive_nesting_handler(false);
Argv_t Function_t::call_stack("rwsh.excessive_nesting");

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
  
// code to call rwsh.excessive_nesting, separated out of operator() for clarity.
void Function_t::excessive_nesting_handler(const Argv_t& src_argv) {
  excessive_nesting = false;
  Argv_t call_stack_copy = call_stack;                     //need for a copy: 
  call_stack.clear();
  call_stack.push_back("rwsh.excessive_nesting");
  if (in_excessive_nesting_handler) {
    Argv_t blank;
    echo_bi(Argv_t("echo rwsh.excessive_nesting itself exceeded MAX_NESTING:"));
    newline_bi(blank);
    echo_bi(call_stack_copy);
    newline_bi(blank);
    echo_bi(Argv_t("echo original call stack:"));
    newline_bi(blank);
    echo_bi(src_argv);
    newline_bi(blank);}
  else {
    in_excessive_nesting_handler = true;
    executable_map[call_stack_copy](call_stack_copy);
    in_excessive_nesting_handler = false;}}
// need for a copy: if rwsh.excessive_nesting exceeds MAX_NESTING itself
//     then it will unwind the stack and write to call_stack. To preserve the 
//     original call stack, we need a copy of call_stack to be the argument.

// run the given function
int Function_t::operator() (const Argv_t& src_argv) {
  if (global_nesting > max_nesting) {
    excessive_nesting = true;
    call_stack.push_back(src_argv[0]);
    return -1;}
  ++current_nesting;
  ++global_nesting;
  for (std::vector<Arg_script_t>::const_iterator i = script.begin();
       i != script.end(); ++i) {
    Argv_t dest_argv = i->interpret(src_argv);
    last_return = (executable_map[dest_argv])(dest_argv);
    if (excessive_nesting) break;}
  --global_nesting;
  --current_nesting;
  if (excessive_nesting) {
    call_stack.push_back(src_argv[0]);
    if (!global_nesting) excessive_nesting_handler(src_argv);}
  int ret = last_return;
  if (del_on_term && !current_nesting) delete this;
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

