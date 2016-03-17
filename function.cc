// The definition of the Function class which can be used to change the 
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2006-2016 Samuel Newbold

#include <iterator>
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
Command_block* Command_block::apply(const Argm& argm, unsigned nesting) const {
  if ((*this)[0].is_argfunction())
    if (argm.argfunction()) {
      Command_block* result = new Command_block(argm.argfunction()->body);
      result->promote_soons(nesting);
      return result;}
    else return 0;
  else {
    Command_block* result = new Command_block();
    std::back_insert_iterator<std::vector<Arg_script> > ins(*result);
    for (Command_block::const_iterator i = begin(); i != end(); ++i) {
      i->apply(argm, nesting, ins);}
    result->trailing = trailing;
    return result;}}
  
int Command_block::operator() (const Argm& src_argm) {
  try {
    if (increment_nesting(src_argm)) return Variable_map::dollar_question;
    int ret = internal_execute(src_argm);
    if (decrement_nesting(src_argm)) ret = Variable_map::dollar_question;
    return ret;}
  catch (Signal_argm error) {
    caught_signal = error.signal;
    std::copy(error.begin(), error.end(), std::back_inserter(call_stack));
    decrement_nesting(src_argm);
    return -1;}}

int Command_block::internal_execute(const Argm& src_argm) {
  int ret;
  for (const_iterator i = begin(); i != end(); ++i) {
    Argm statement_argm = i->interpret(src_argm);
    ret = executable_map.run(statement_argm);
    if (unwind_stack()) break;}
  return ret;}

void Command_block::promote_soons(unsigned nesting) {
  if (!this) return;
  for (iterator i = begin(); i != end(); ++i) i->promote_soons(nesting);}

std::string Command_block::str() const {
  std::string body;
  for (const_iterator i = begin(); i != end()-1; ++i) body += i->str() + "";
  return "{" + body + back().str() + "}";}  //+ trailing + ",";}

std::string Parameter_group::str() const {
  if (required) return names[0] + (elipsis? "": " ...");
  else {
    std::string result("[");
    if (elipsis == -1) result.append("... ");
    for (std::vector<std::string>::size_type j = 0; j < names.size(); ++j)
       result.append((j? " ": "") + names[j] + (elipsis == j? " ...": ""));
    return result + "]";}}

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

// constructor that parses prototypes
Function::Function(const std::string& name_i,
                   Argm::const_iterator first_parameter,
                   Argm::const_iterator parameter_end,
                   bool non_prototype_i,
                   Flag_type flags_i,
                   const Command_block& src) :
    prototype(first_parameter, parameter_end, non_prototype_i, flags_i),
    name_v(name_i), body(src) {}

// generate a new function by unescaping argument functions and replacing
// unescaped_argfunction with the argument function in argm
Function* Function::apply(const Argm& argm, unsigned nesting) const {
  if (body[0].is_argfunction())
    if (argm.argfunction()) {
      Function* result = new Function(*argm.argfunction());
      result->promote_soons(nesting);
      return result;}
    else return 0;
  else {
    Function* result = new Function(name());
    std::back_insert_iterator<std::vector<Arg_script> > ins(result->body);
    for (Command_block::const_iterator i = body.begin(); i != body.end(); ++i) {
      i->apply(argm, nesting, ins);}
    return result;}}
  
// run the given function
int Function::operator() (const Argm& invoking_argm) { 
  try {
    if (increment_nesting(invoking_argm)) return Variable_map::dollar_question;
    struct timeval start_time;
    gettimeofday(&start_time, rwsh_clock.no_timezone);
    ++execution_count_v;
    Variable_map locals_map(prototype.arg_to_param(invoking_argm));
    Argm interpreted_argm(invoking_argm.begin(), invoking_argm.end(),
               invoking_argm.argfunction(), &locals_map,
               invoking_argm.input, invoking_argm.output, invoking_argm.error);
    int ret = body.internal_execute(interpreted_argm);
    last_return = ret;
    struct timeval end_time;
    gettimeofday(&end_time, rwsh_clock.no_timezone);
    last_execution_time_v = Clock::timeval_sub(end_time, start_time);
    Clock::timeval_add(total_execution_time_v, last_execution_time_v);
    if (decrement_nesting(invoking_argm)) ret = Variable_map::dollar_question;
    return ret;}
  catch (Signal_argm error) {
    caught_signal = error.signal;
    std::copy(error.begin(), error.end(), std::back_inserter(call_stack));
    decrement_nesting(invoking_argm);
    return -1;}}

void Function::promote_soons(unsigned nesting) {
  if (!this) return;
  else body.promote_soons(nesting);}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function::str() const {
  return prototype.str(name()) + body.str();}
