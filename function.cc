// The definition of the Function_t class which can be used to change the 
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "tokenize.cc"
#include "variable_map.h"

Function_t::Function_t(const std::string& name_i, const std::string& src, 
      unsigned max_soon) : name_v(name_i) {
  std::vector<std::string> commands;
  tokenize_same_brace_level(src, std::back_inserter(commands), 
                            std::bind2nd(std::equal_to<char>(), ';'));
  for (std::vector<std::string>::const_iterator i = commands.begin();
       i != commands.end(); ++i) {
    script.push_back(Arg_script_t(*i, max_soon));
    if (commands.size() != 1 && script.back().is_argfunction())
      Rwsh_stream_p() <<"rwsh.argfunction cannot occur as one of several "
                  "commands\n";}}
  
// generate a new function by unescaping argument functions and replacing
// unescaped_argfunction with the argument function in argv
Function_t* Function_t::apply(const Argv_t& argv, unsigned nesting) const {
  if (script[0].is_argfunction()) 
    return argv.argfunction()->promote_soons(nesting);
  else {
    Function_t* result = new Function_t(name());
    std::back_insert_iterator<std::vector<Arg_script_t> > ins(result->script);
    for (const_iterator i = script.begin(); i != script.end(); ++i) {
      i->apply(argv, nesting, ins);}
    return result;}}
  
// run the given function
int Function_t::operator() (const Argv_t& src_argv) { 
  if (increment_nesting(src_argv)) return dollar_question;
  int ret;
  for (const_iterator i = script.begin(); i != script.end(); ++i) {
    Argv_t dest_argv;
    try {dest_argv = i->interpret(src_argv);}
    catch (Failed_substitution_t error) {dest_argv = error;}
    catch (Undefined_variable_t error) {break;}
    ret = executable_map.run(dest_argv);
    if (unwind_stack()) break;}
  last_return = ret;
  if (decrement_nesting(src_argv)) ret = dollar_question;
  return ret;}

Function_t* Function_t::promote_soons(unsigned nesting) const {
  if (!this) return 0;
  Function_t* result = new Function_t(name_v, script);
  for (iterator i = result->script.begin(); i != result->script.end(); ++i) 
    i->promote_soons(nesting);
  return result;}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function_t::str() const {
    std::string body;
    for (const_iterator i = script.begin(); i != script.end()-1; ++i) 
      body += i->str() + "; ";
    body += script.back().str();
    if (name() == "rwsh.argfunction") return "{" + body + "}";
    else return "%function " + name_v + " {" + body + "}";}

