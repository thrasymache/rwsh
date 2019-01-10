// The definition of the Function class which can be used to change the
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2006-2018 Samuel Newbold

#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "call_stack.h"
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

// generate a new Command_block by unescaping argument functions and replacing
// unescaped_argfunction with the argument function in argm
Command_block* Command_block::apply(const Argm& argm, unsigned nesting,
                                    Error_list& exceptions) const {
  if ((*this)[0].is_argfunction())
    if (argm.argfunction()) {
      Command_block* result = new Command_block(*argm.argfunction());
      result->promote_soons(nesting);
      return result;}
    else return 0;
  else {
    Command_block* result = new Command_block();
    std::back_insert_iterator<std::vector<Arg_script> > ins(*result);
    for (auto i: *this) i.apply(argm, nesting, ins, exceptions);
    result->trailing = trailing;
    return result;}}

void Command_block::execute(const Argm& src_argm,
                           Error_list& exceptions) const {
  for (auto j: *this) {
    Argm statement_argm = j.interpret(src_argm, exceptions);
    if (global_stack.unwind_stack()) break;
    executable_map.run(statement_argm, exceptions);
    if (global_stack.unwind_stack()) break;}}

void Command_block::prototype_execute(const Argm& argm,
                                     const Prototype& prototype,
                                     Error_list& exceptions) const {
  Variable_map locals(prototype.arg_to_param(argm));
  Argm params(argm.argv(), argm.argfunction(), &locals,
              argm.input, argm.output, argm.error);
  try {
    execute(params, exceptions);
    prototype.unused_var_check(&locals, exceptions);}
  catch (Exception error) {
    prototype.unused_var_check(&locals, exceptions);
    throw error;}}

void Command_block::promote_soons(unsigned nesting) {
  if (this) for (auto j = begin(); j != end(); ++j) j->promote_soons(nesting);}

std::string Command_block::str() const {
  std::string body;
  for (auto i = begin(); i != end()-1; ++i) body += i->str() + "";
  return "{" + body + back().str() + "}";}  //+ trailing + ",";}

Command_block::Command_block(const std::string& src,
                                  std::string::size_type& point,
                                  unsigned max_soon, Error_list& errors) {
  std::string::size_type tpoint = point;
  while (tpoint != std::string::npos && src[tpoint] != '}') {
    push_back(Arg_script(src, ++tpoint, max_soon, errors));
    if (size() != 1 && back().is_argfunction())
      default_output <<".argfunction cannot occur as one of several "
                  "commands\n";}
  if (tpoint == std::string::npos)
    throw Unclosed_brace(src.substr(0, point-1));
  if (!size()) push_back(Arg_script("", max_soon, errors));
  point = tpoint + 1;}

Function::Function(const std::string& name_i,
                   Argm::const_iterator first_parameter,
                   Argm::const_iterator parameter_end,
                   bool non_prototype_i,
                   const std::string& src, Error_list& errors) :
    name_v(name_i),
    prototype(first_parameter, parameter_end, non_prototype_i) {
  if (prototype.non_prototype) default_error <<"deprecated string non-prototype: " <<name_i <<"\n";
  std::string::size_type point = 0;
  try {
    body = Command_block(src, point, 0, errors);
    // this error handling is not simply testable because it requires bad
    // functions in rwsh_init.cc
    if (point != src.length())
      throw "function with text following closing brace " + name_i + "\n" +
          src.substr(point) + "\n";}
  catch (Unclosed_brace error) {
    throw "unclosed brace on construction of function " + name_i + "\n" +
      error[1] + "\n";}
  catch (Unclosed_parenthesis error) {
    throw "unclosed parenthesis on construction of function " + name_i + "\n" +
      error[1] + "\n";}}

Function::Function(const std::string& name_i,
                   Argm::const_iterator first_parameter,
                   Argm::const_iterator parameter_end,
                   bool non_prototype_i,
                   const Command_block& src) :
    prototype(first_parameter, parameter_end, non_prototype_i),
    name_v(name_i), body(src) {
  //if (non_prototype_i) default_error <<"re-deprecated .function non-prototype: " <<name_i <<"\n";
  }

// run the given function
void Function::execute(const Argm& argm, Error_list& exceptions) const {
  body.prototype_execute(argm, prototype, exceptions);}

void Function::promote_soons(unsigned nesting) {
  if (this) body.promote_soons(nesting);}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function::str() const {
  if (prototype.non_prototype)
    return ".function " + escape(name()) + " " + body.str();
  else return ".function_all_flags " + escape(name()) +
              " " + prototype.str() + " " + body.str();}
