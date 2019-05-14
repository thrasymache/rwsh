// The definition of the Builtin, Command_block, and Function classes. The
// first of which executes commands that are implemented by functions in
// builtin.cc, the latter of which map arguments passed to an executable
// and/or tie several other executables into a single executable.
//
// Copyright (C) 2006-2019 Samuel Newbold

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

Builtin::Builtin(const std::string& name_i,
                 void (*impl)(const Argm& argm, Error_list& exceptions),
                 const Argv& prototype_i) :
  implementation(impl), name_v(name_i), prototype(prototype_i) {}

// run the given builtin
void Builtin::execute(const Argm& argm, Error_list& exceptions) {
  Variable_map locals(argm.parent_map());
  prototype.arg_to_param(argm.argv(), locals, exceptions);
  locals.bless_unused_vars();
  if (argm.argfunction() && prototype.exclude_argfunction)
    exceptions.add_error(Exception(Argm::Excess_argfunction));
  else if (!argm.argfunction() && prototype.required_argfunction)
    exceptions.add_error(Exception(Argm::Missing_argfunction));
  if (!global_stack.unwind_stack())
    (*implementation)(argm, exceptions);}

std::string Builtin::str() const {return name_v + " " + prototype.str();};

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

void Command_block::execute(const Argm& argm, Error_list& exceptions) {
  Prototype prototype(Argv{});
  Variable_map locals(argm.parent_map());
  try {
    Argm params(argm.argv(), argm.argfunction(), &locals,
                argm.input, argm.output, argm.error);
    statements_execute(params, exceptions);
    prototype.unused_var_check(&locals, exceptions);}
  catch (Exception error) {
    prototype.unused_var_check(&locals, exceptions);
    throw error;}}

void Command_block::statements_execute(const Argm& src_argm,
                                       Error_list& exceptions) {
  for (auto j: *this) {
    Argm statement_argm = j.interpret(src_argm, exceptions);
    if (global_stack.unwind_stack()) break;
    executable_map.run(statement_argm, exceptions);
    if (global_stack.unwind_stack()) break;}}

void Command_block::prototype_execute(const Argm& argm,
                                     const Prototype& prototype,
                                     Error_list& exceptions) {
  Variable_map locals(argm.parent_map());
  prototype.arg_to_param(argm.argv(), locals, exceptions);
  if (argm.argfunction() && prototype.exclude_argfunction)
    exceptions.add_error(Exception(Argm::Excess_argfunction));
  else if (!argm.argfunction() && prototype.required_argfunction)
    exceptions.add_error(Exception(Argm::Missing_argfunction));
  try {
    if (!global_stack.unwind_stack()) {
      Argm params(argm.argv(), argm.argfunction(), &locals,
                  argm.input, argm.output, argm.error);
      statements_execute(params, exceptions);}
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

// if src[point] == '{' , then the block ends at the matching '}', otherwise
// it continues to the end of the string.
Command_block::Command_block(const std::string& src,
                                  std::string::size_type& point,
                                  unsigned max_soon, Error_list& errors) {
  std::string::size_type tpoint = point;
  while (tpoint != std::string::npos) {
    if (src[tpoint] == '}')
      if (src[point] == '{') break;
      else errors.add_error(Exception(Argm::Mismatched_brace,
                                      src.substr(0, tpoint+1)));
    else;
    if (src[tpoint] == '{' || src[tpoint] == '}' ||
        src[tpoint] == ';' || src[tpoint] =='\n')  ++tpoint;
    push_back(Arg_script(src, tpoint, max_soon, errors));
    if (size() != 1 && back().is_argfunction())
      default_output <<".argfunction cannot occur as one of several "
                  "commands\n";}
  if (tpoint == std::string::npos) {
    if (src[point] == '{') errors.add_error(Exception(Argm::Unclosed_brace,
                                                      src.substr(0, point-1)));
    point = std::string::npos;}
  else point = tpoint + 1;}

Function::Function(const std::string& name_i, const Argv& parameters,
                   const Command_block& src) :
     name_v(name_i), prototype(parameters), body(src) {}

// run the given function
void Function::execute(const Argm& argm, Error_list& exceptions) {
  body.prototype_execute(argm, prototype, exceptions);}

void Function::promote_soons(unsigned nesting) {
  if (this) body.promote_soons(nesting);}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function::str() const {
  return ".function " + escape(name()) + " " + prototype.str() + " " +
    body.str();}
