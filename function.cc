// The definition of the Function class which can be used to change the 
// arguments passed to an executable and/or tie several other executables into
// a single executable.
//
// Copyright (C) 2006-2015 Samuel Newbold

#include <iterator>
#include <map>
#include <set>
#include <string>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argm.h"
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
    name_v(name_i), positional(), required_argc(0), flag_options(),
    parameter_names(), positional_parameters(true) {
  internal_constructor(src, point, max_soon);}

Function::Function(const std::string& name_i, const std::string& src) :
    name_v(name_i), positional(), required_argc(0), flag_options(),
    parameter_names(), positional_parameters(true) {
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

// constructor that parses parameters
Function::Function(const std::string& name_i,
                   Argm::const_iterator first_parameter,
                   Argm::const_iterator parameter_end,
                   bool positional_parameters_i,
                   const std::vector<Arg_script>& src) :
    name_v(name_i), positional(), required_argc(), flag_options(),
    parameter_names(), positional_parameters(positional_parameters_i),
    script(src) {
  for (Argm::const_iterator i = first_parameter; i != parameter_end; ++i)
    if ((*i)[0] == '[') {
      if ((*i)[i->length()-1] != ']')
        throw Signal_argm(Argm::Mismatched_bracket, *i);
      std::string name(i->substr(1, i->length()-2));
      if (parameter_names.find(name) != parameter_names.end())
        throw Signal_argm(Argm::Duplicate_parameter, name);
      parameter_names.insert(name);
      if (name[0] != '-') positional.push_back(Parameter(false, name));
      else if (name == "--"); // discard implicit [--]
      else flag_options.insert(name);}
    else if (parameter_names.find(*i) != parameter_names.end() || *i == "--")
      throw Signal_argm(Argm::Duplicate_parameter, *i);
    else {
      parameter_names.insert(*i);
      positional.push_back(Parameter(true, *i));
      ++required_argc;}}

// generate a new function by unescaping argument functions and replacing
// unescaped_argfunction with the argument function in argm
Function* Function::apply(const Argm& argm, unsigned nesting) const {
  if (script[0].is_argfunction()) 
    return argm.argfunction()->promote_soons(nesting);
  else {
    Function* result = new Function(name());
    std::back_insert_iterator<std::vector<Arg_script> > ins(result->script);
    for (const_iterator i = script.begin(); i != script.end(); ++i) {
      i->apply(argm, nesting, ins);}
    return result;}}
  
// run the given function
int Function::operator() (const Argm& invoking_argm) { 
  try {
    if (increment_nesting(invoking_argm)) return Variable_map::dollar_question;
    struct timeval start_time;
    gettimeofday(&start_time, rwsh_clock.no_timezone);
    ++execution_count_v;
    Variable_map locals_map(invoking_argm.parent_map());
    Argm interpreted_argm(invoking_argm.begin(), invoking_argm.end(),
               invoking_argm.argfunction(), &locals_map,
               invoking_argm.input, invoking_argm.output, invoking_argm.error);
    if (!positional_parameters) {
      if (flag_options.begin() != flag_options.end())
        locals_map.local("-*", "");
      unsigned available = invoking_argm.argc()-1;
      Argm::const_iterator i = invoking_argm.begin()+1;
      for (std::set<std::string>::const_iterator j;
           i != invoking_argm.end() &&
               (j = flag_options.find(*i)) != flag_options.end();
           ++i, --available) {
        std::string current_options = locals_map.get("-*");
        if (current_options == "") locals_map.set("-*", *i);
        else locals_map.set("-*", current_options + " " + *i);
        if (!locals_map.exists(*j)) locals_map.local(*j, *i);
        else locals_map.set(*j, locals_map.get(*j) + " " + *i);}
      if (i != invoking_argm.end() && (*i)[0] == '-')
        if(*i == "--") {                                       // "discard" --
          locals_map.local(*i, *i);
          if (locals_map.exists("-*")) {
            std::string current_options = locals_map.get("-*");
            if (current_options == "") locals_map.set("-*", *i);
            else locals_map.set("-*", current_options + " " + *i);}
          ++i;}
        else throw Signal_argm(Argm::Unrecognized_flag, *i);
      unsigned required_remaining = required_argc;
      std::vector<Parameter>::const_iterator j = positional.begin();
      if (available < required_remaining) {
        unsigned non_optional = available + required_argc - required_remaining;
        throw Signal_argm(Argm::Bad_argc, non_optional, required_argc,
                          invoking_argm.argc()-1-non_optional);}
      for (; i != invoking_argm.end() && j != positional.end(); ++i, ++j) {
        if (j->required) {
          --required_remaining;
          locals_map.local(j->name, *i);}
        else if (available > required_remaining) locals_map.local(j->name, *i);
        else {
          unsigned non_optional = available +required_argc -required_remaining;
          throw Signal_argm(Argm::Bad_argc, non_optional, required_argc,
                            invoking_argm.argc()-1-non_optional);}
        --available;}
      if (i != invoking_argm.end()) {
        unsigned non_optional = required_argc;
        while (i != invoking_argm.end()) ++non_optional, ++i;
        throw Signal_argm(Argm::Bad_argc, required_argc+1, required_argc,
                          invoking_argm.argc()-1-non_optional);}}
    int ret;
    for (const_iterator i = script.begin(); i != script.end(); ++i) {
      Argm statement_argm = i->interpret(interpreted_argm);
      ret = executable_map.run(statement_argm);
      if (unwind_stack()) break;}
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

Function* Function::promote_soons(unsigned nesting) const {
  if (!this) return 0;
  Function* result = new Function(*this);
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

