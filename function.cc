// The definition of the Function_t class which can be used to change the 
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

Function_t::Function_t(const std::string& name_i, const std::string& src,
                       std::string::size_type& point, unsigned max_soon) :
    name_v(name_i) {
  std::string::size_type tpoint = point;
  while (tpoint != std::string::npos && src[tpoint] != '}') {
    script.push_back(Arg_script_t(src, ++tpoint, max_soon));
    if (script.size() != 1 && script.back().is_argfunction())
      default_output <<"rwsh.argfunction cannot occur as one of several "
                  "commands\n";}
  if (tpoint == std::string::npos)
    throw Unclosed_brace_t(src.substr(0, point-1));
  if (!script.size()) script.push_back(Arg_script_t("", max_soon));
  point = tpoint + 1;}

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
  struct timeval start_time;
  gettimeofday(&start_time, rwsh_clock.no_timezone);
  ++execution_count_v;
  int ret;
  for (const_iterator i = script.begin(); i != script.end(); ++i) {
    Argv_t dest_argv;
    try {dest_argv = i->interpret(src_argv);}
    catch (Failed_substitution_t error) {break;}
    catch (Undefined_variable_t error) {break;}
    ret = executable_map.run(dest_argv);
    if (unwind_stack()) break;}
  last_return = ret;
  struct timeval end_time;
  gettimeofday(&end_time, rwsh_clock.no_timezone);
  last_execution_time_v = Clock::timeval_sub(end_time, start_time);
  Clock::timeval_add(total_execution_time_v, last_execution_time_v);
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
    return "{" + body + script.back().str() + "}";}

