// The definition of the Function class which can be used to change the
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2006-2016 Samuel Newbold

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
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

// generate a new Command_block by unescaping argument functions and replacing
// unescaped_argfunction with the argument function in argm
Command_block* Command_block::apply(const Argm& argm, unsigned nesting,
                                    std::list<Argm>& exceptions) const {
  if ((*this)[0].is_argfunction())
    if (argm.argfunction()) {
      Command_block* result = new Command_block(*argm.argfunction());
      result->promote_soons(nesting);
      return result;}
    else return 0;
  else {
    Command_block* result = new Command_block();
    std::back_insert_iterator<std::vector<Arg_script> > ins(*result);
    for (Command_block::const_iterator i = begin(); i != end(); ++i) {
      i->apply(argm, nesting, ins, exceptions);}
    result->trailing = trailing;
    return result;}}

int Command_block::collect_errors_core(const Argm& src_argm,
                                   const std::vector<std::string>& exceptional,
                                   bool logic,
                                   std::list<Argm>& parent_exceptions) {
  int ret;
  for (const_iterator i = begin(); i != end() && !unwind_stack_v; ++i) {
    if (current_exception_count > max_collect) {
      if (!collect_excess_thrown)
        add_error(parent_exceptions,
                  Exception(Argm::Excessive_exceptions_collected, max_collect));
      unwind_stack_v = collect_excess_thrown = true;
      return ret;}
    std::list<Argm> children;
    Argm statement_argm = i->interpret(src_argm, children);
    ret = executable_map.run(statement_argm, children);
    if (children.size()) {
      unwind_stack_v = false;
      for (std::list<Argm>::iterator i = children.begin();
           i != children.end();) {
        if (logic == (find(exceptional.begin(), exceptional.end(),
                          (*i)[0]) != exceptional.end()))
          unwind_stack_v = true;
        parent_exceptions.push_back(*i++);}}}
  if (parent_exceptions.size()) unwind_stack_v = true;
  return ret;}

int Command_block::execute(const Argm& src_argm,
                           std::list<Argm>& exceptions) const {
  int ret;
  for (const_iterator i = begin(); i != end(); ++i) {
    Argm statement_argm = i->interpret(src_argm, exceptions);
    ret = executable_map.run(statement_argm, exceptions);
    if (unwind_stack()) break;}
  return ret;}

int Command_block::prototype_execute(const Argm& argm,
                                     const Prototype& prototype,
                                     std::list<Argm>& exceptions) const {
  Variable_map locals(prototype.arg_to_param(argm));
  Argm params(argm.begin(), argm.end(), argm.argfunction(), &locals,
              argm.input, argm.output, argm.error);
  return execute(params, exceptions);}

void Command_block::promote_soons(unsigned nesting) {
  if (this)
    for (iterator i = begin(); i != end(); ++i) i->promote_soons(nesting);}

std::string Command_block::str() const {
  std::string body;
  for (const_iterator i = begin(); i != end()-1; ++i) body += i->str() + "";
  return "{" + body + back().str() + "}";}  //+ trailing + ",";}

Command_block::Command_block(const std::string& src,
                                  std::string::size_type& point,
                                  unsigned max_soon) {
  std::string::size_type tpoint = point;
  while (tpoint != std::string::npos && src[tpoint] != '}') {
    push_back(Arg_script(src, ++tpoint, max_soon));
    if (size() != 1 && back().is_argfunction())
      default_output <<"rwsh.argfunction cannot occur as one of several "
                  "commands\n";}
  if (tpoint == std::string::npos)
    throw Unclosed_brace(src.substr(0, point-1));
  if (!size()) push_back(Arg_script("", max_soon));
  point = tpoint + 1;}

Function::Function(const std::string& name_i, const std::string& src,
                   std::string::size_type& point, unsigned max_soon) :
    name_v(name_i), prototype(true), body(src, point, max_soon) {}

Function::Function(const std::string& name_i, const std::string& src) :
    name_v(name_i), prototype(true) {
  std::string::size_type point = 0;
  try {
    body = Command_block(src, point, 0);
    if (point != src.length())
      throw "function with text following closing brace " + name_i + "\n" +
          src.substr(point) + "\n";}
  catch (Unclosed_brace error) {
    throw "unclosed brace on construction of function " + name_i + "\n" +
      error.prefix + "\n";}
  catch (Unclosed_parenthesis error) {
    throw "unclosed parenthesis on construction of function " + name_i + "\n" +
      error.prefix + "\n";}}

Function::Function(const std::string& name_i,
                   Argm::const_iterator first_parameter,
                   Argm::const_iterator parameter_end,
                   bool non_prototype_i,
                   const Command_block& src) :
    prototype(first_parameter, parameter_end, non_prototype_i),
    name_v(name_i), body(src) {}

// run the given function
int Function::execute(const Argm& argm, std::list<Argm>& exceptions) const {
  return body.prototype_execute(argm, prototype, exceptions);}

void Function::promote_soons(unsigned nesting) {
  if (this) body.promote_soons(nesting);}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function::str() const {
  if (prototype.non_prototype)
    return ".function " + escape(name()) + " " + body.str();
  else return ".function_all_flags " + escape(name()) +
              " " + prototype.str() + " " + body.str();}
