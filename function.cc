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

namespace {
  struct Statement_terminator : public std::unary_function<char, bool> {
    bool operator() (const char& x) const {return x == ';' || x =='\n';} };

std::string::size_type find_close_brace(const std::string& focus,
                                        std::string::size_type i) {
  unsigned nesting = 1;
  while (nesting && (i = focus.find_first_of("{}", i+1)) != std::string::npos) {
    if (focus[i] == '{') ++nesting;
    else --nesting;}
  return i;}

// only tokenizes text that is not within brace pairs.
// repeated separators result in empty tokens.
// empty input produces a single empty string
template<class Out, class Pred>
Out tokenize_same_brace_level(const std::string& in, Out res, Pred p) {
  unsigned token_start=0, i=0, brace_level=0;
  for (; i<in.length(); ++i)
    if (in[i] == '{') ++brace_level;
    else if (in[i] == '}') --brace_level;
    else if (!brace_level && p(in[i])) {
      *res++ = in.substr(token_start, i-token_start);
      token_start = i + 1;}
  *res = in.substr(token_start, i-token_start);
  return res;}

} // end unnamed namespace

Function_t::Function_t(const std::string& name_i, const std::string& function,
                       unsigned max_soon) :
    name_v(name_i) {
  std::vector<std::string> commands;
  tokenize_same_brace_level(function, std::back_inserter(commands), 
                            Statement_terminator());
  for (std::vector<std::string>::const_iterator i = commands.begin();
       i != commands.end(); ++i) {
    script.push_back(Arg_script_t(*i, max_soon));
    if (commands.size() != 1 && script.back().is_argfunction())
      default_output <<"rwsh.argfunction cannot occur as one of several "
                  "commands\n";}}
  
Function_t::Function_t(const std::string& name_i, const std::string& src,
                       std::string::size_type& point, unsigned max_soon) :
    name_v(name_i) {
  std::string::size_type close_brace = find_close_brace(src, point);
  if (close_brace == std::string::npos)
    throw Unclosed_brace_t(src.substr(0, point+1));
  std::string f_str = src.substr(point+1, close_brace-point-1); 
  std::vector<std::string> commands;
  tokenize_same_brace_level(f_str, std::back_inserter(commands), 
                            Statement_terminator());
  for (std::vector<std::string>::const_iterator i = commands.begin();
       i != commands.end(); ++i) {
    script.push_back(Arg_script_t(*i, max_soon));
    if (commands.size() != 1 && script.back().is_argfunction())
      default_output <<"rwsh.argfunction cannot occur as one of several "
                  "commands\n";}
  point = close_brace + 1;}
  
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
    catch (Failed_substitution_t error) {dest_argv = error;}
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
    body += script.back().str();
    if (name() == "rwsh.argfunction") return "{" + body + "}";
    else return "%function " + name_v + " {" + body + "}";}

