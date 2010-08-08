// The definition of the Function class which can be used to change the 
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <iterator>
#include <map>
#include <string>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "variable_map.h"

void Function::internal_constructor(const std::string& src,
                             std::string::size_type& point, unsigned max_soon) {
  std::string::size_type tpoint = point;
  while (tpoint != std::string::npos && src[tpoint] != '}') {
    script.push_back(Arg_script(src, ++tpoint, max_soon));
    if (script.size() != 1 && script.back().is_argfunction())
      default_output <<"rwsh.argfunction cannot occur as one of several "
                  "commands\n";}
  if (tpoint == std::string::npos)
    throw Unclosed_brace(src.substr(0, point-1));
  if (!script.size()) script.push_back(Arg_script("", max_soon));
  point = tpoint + 1;}

Function::Function(const std::string& name_i, const std::string& src,
                   std::string::size_type& point, unsigned max_soon) :
    name_v(name_i) {internal_constructor(src, point, max_soon);}

Function::Function(const std::string& name_i, const std::string& src) :
    name_v(name_i) {
  std::string::size_type point = 0;
  try {
    internal_constructor(src, point, 0);
    if (point != src.length())
      throw "function with text following closing brace " + name_i + "\n" +
          src.substr(point) + "\n";}
  catch (Unclosed_brace error) {
    throw "unclosed brace on construction of function " + name_i + "\n" +
      error.prefix + "\n";}
  catch (Unclosed_parenthesis error) {
    throw "unclosed parenthesis on construction of function " + name_i + "\n" +
      error.prefix + "\n";}}

// generate a new function by unescaping argument functions and replacing
// unescaped_argfunction with the argument function in argv
Function* Function::apply(const Argv& argv, unsigned nesting) const {
  if (script[0].is_argfunction()) 
    return argv.argfunction()->promote_soons(nesting);
  else {
    Function* result = new Function(name());
    std::back_insert_iterator<std::vector<Arg_script> > ins(result->script);
    for (const_iterator i = script.begin(); i != script.end(); ++i) {
      i->apply(argv, nesting, ins);}
    return result;}}
  
// run the given function
int Function::operator() (const Argv& src_argv) { 
try {
  if (increment_nesting(src_argv)) return dollar_question;
  struct timeval start_time;
  gettimeofday(&start_time, rwsh_clock.no_timezone);
  ++execution_count_v;
  int ret;
  for (const_iterator i = script.begin(); i != script.end(); ++i) {
    Argv dest_argv;
    try {dest_argv = i->interpret(src_argv);}
    catch (Signal_argv error) {throw;}
    ret = executable_map.run(dest_argv);
    if (unwind_stack()) break;}
  last_return = ret;
  struct timeval end_time;
  gettimeofday(&end_time, rwsh_clock.no_timezone);
  last_execution_time_v = Clock::timeval_sub(end_time, start_time);
  Clock::timeval_add(total_execution_time_v, last_execution_time_v);
  if (decrement_nesting(src_argv)) ret = dollar_question;
  return ret;}
  catch (Signal_argv error) {
    caught_signal = true;
    std::copy(error.begin(), error.end(), std::back_inserter(call_stack));
    decrement_nesting(src_argv);
    return -1;}}

Function* Function::promote_soons(unsigned nesting) const {
  if (!this) return 0;
  Function* result = new Function(name_v, script);
  for (iterator i = result->script.begin(); i != result->script.end(); ++i) 
    i->promote_soons(nesting);
  return result;}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function::str() const {
    std::string body;
    for (const_iterator i = script.begin(); i != script.end()-1; ++i) 
      body += i->str() + "; ";
    return "{" + body + script.back().str() + "}";}

