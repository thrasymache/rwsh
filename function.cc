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
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "clock.h"
#include "executable.h"
#include "executable_map.h"
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
  for (const_iterator i = begin(); i != end()-1; ++i) body += i->str() + "; ";
  return "{" + body + back().str() + "}";}

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
    name_v(name_i), positional(), required_argc(0), flag_options(),
    parameter_names(), positional_parameters(true), all_flags(true),
    body(src, point, max_soon) {}

Function::Function(const std::string& name_i, const std::string& src) :
    name_v(name_i), positional(), required_argc(0), flag_options(),
    parameter_names(), positional_parameters(true), all_flags(true) {
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

// constructor that parses parameters
Function::Function(const std::string& name_i,
                   Argm::const_iterator first_parameter,
                   Argm::const_iterator parameter_end,
                   bool positional_parameters_i,
                   bool all_flags_i,
                   const Command_block& src) :
    name_v(name_i), positional(), required_argc(), flag_options(),
    parameter_names(), positional_parameters(positional_parameters_i),
    all_flags(all_flags_i), body(src) {
  for (Argm::const_iterator i = first_parameter; i != parameter_end; ++i)
    if ((*i)[0] == '[')
      if ((*i)[i->length()-1] == ']') {
        std::string param_name(i->substr(1, i->length()-2));
        if (parameter_names.find(param_name) != parameter_names.end())
          throw Signal_argm(Argm::Duplicate_parameter, param_name);
        parameter_names.insert(param_name);
        if (param_name[0] != '-')
          positional.push_back(Parameter_group(false, param_name));
        else if (param_name == "--"); // discard implicit [--]
        else flag_options[param_name] = Parameter_group(false, param_name);}
      else {
        std::string param_name(i->substr(1, i->length()-1));
        Parameter_group group(false, param_name);
        for (++i; i != parameter_end && (*i)[i->length()-1] != ']'; ++i)
          group.names.push_back(*i);
        if (i != parameter_end)
          group.names.push_back(i->substr(0, i->length()-1));
        else throw Signal_argm(Argm::Mismatched_bracket, "[" + param_name);
        flag_options[param_name] = group;}
    else if (parameter_names.find(*i) != parameter_names.end() || *i == "--")
      throw Signal_argm(Argm::Duplicate_parameter, *i);
    else {
      parameter_names.insert(*i);
      positional.push_back(Parameter_group(true, *i));
      ++required_argc;}}

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
    Variable_map locals_map(invoking_argm.parent_map());
    Argm interpreted_argm(invoking_argm.begin(), invoking_argm.end(),
               invoking_argm.argfunction(), &locals_map,
               invoking_argm.input, invoking_argm.output, invoking_argm.error);
    if (!positional_parameters) {
      if (!all_flags || flag_options.begin() != flag_options.end())
        locals_map.local("-*", "");
      unsigned available = invoking_argm.argc()-1;
      Argm::const_iterator i = invoking_argm.begin()+1;
      while (i != invoking_argm.end() && (*i)[0] == '-') {
        std::map<std::string, Parameter_group>::const_iterator j =
                                                         flag_options.find(*i);
        if (j != flag_options.end()) {
          std::string flag = j->second.names[0];
          for (std::vector<std::string>::const_iterator k =
                                                       j->second.names.begin();
               i != invoking_argm.end() && k != j->second.names.end();
               ++k, ++i, --available) {
            locals_map.local_or_append_word(flag, *i);
            if (locals_map.exists("-*"))
              locals_map.set_or_append_word("-*", *i);
            if (*k != flag) locals_map.local_or_append_word(*k, *i);}}
        else {
          if (locals_map.exists("-*")) locals_map.set_or_append_word("-*", *i);
          if(*i == "--") {                                      // "discard" --
            locals_map.local(*i, *i);
            ++i, --available; break;}
          else if (all_flags) throw Signal_argm(Argm::Unrecognized_flag, *i);
          else ++i, --available;}}
      unsigned required_remaining = required_argc;
      std::vector<Parameter_group>::const_iterator j = positional.begin();
      if (available < required_remaining) {
        unsigned non_optional = available + required_argc - required_remaining;
        throw Signal_argm(Argm::Bad_argc, non_optional, required_argc,
                          invoking_argm.argc()-1-non_optional);}
      for (; i != invoking_argm.end() && j != positional.end(); ++i, ++j) {
        if (j->required) {
          --required_remaining;
          locals_map.local(j->names[0], *i);}
        else if (available > required_remaining)
          locals_map.local(j->names[0], *i);
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
  return body.str();}
