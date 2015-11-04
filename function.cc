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
    name_v(name_i), positional(), required_argc(0), flag_options(),
    parameter_names(), non_prototype(true), flags(ALL),
    explicit_dash_dash(false), body(src, point, max_soon) {}

Function::Function(const std::string& name_i, const std::string& src) :
    name_v(name_i), positional(), required_argc(0), flag_options(),
    parameter_names(), non_prototype(true), flags(ALL),
    explicit_dash_dash(false) {
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
    name_v(name_i), positional(), required_argc(), flag_options(),
    parameter_names(), non_prototype(non_prototype_i),
    flags(flags_i), explicit_dash_dash(false), body(src) {
  bool has_elipsis = false;
  for (Argm::const_iterator i = first_parameter; i != parameter_end; ++i)
    if (*i == "..." || *i == "[...]")
      if (!positional.size()) throw Signal_argm(Argm::Elipsis_first_arg, *i);
      else if (has_elipsis) throw Signal_argm(Argm::Post_elipsis_option, *i);
      else positional.back().elipsis = 0, has_elipsis = true;
    else if ((*i)[0] != '[')
      if (*i == "--") throw Signal_argm(Argm::Duplicate_parameter, *i);
      else {
        check_for_duplicates(*i);
        positional.push_back(Parameter_group(true));
        positional.back().names.push_back(*i);
        ++required_argc;}
    else {
      if (has_elipsis) throw Signal_argm(Argm::Post_elipsis_option, *i);
      bool group_end((*i)[i->length()-1] == ']');
      std::string first_name(i->substr(1, i->length() - 1 - group_end));
      Parameter_group group(false);
      if (first_name == "...") group.elipsis = -1, has_elipsis = true;
      else group.names.push_back(first_name);
      while (!group_end && ++i != parameter_end) {
        group_end = (*i)[i->length()-1] == ']';
        std::string name(i->substr(0, i->length() - group_end));
        if (name == "...")
          if (has_elipsis) throw Signal_argm(Argm::Post_elipsis_option, name);
          else group.elipsis = group.names.size()-1, has_elipsis = true;
        else group.names.push_back(name);}
      if (!group_end) {
        std::string group_str(group.str());
        throw Signal_argm(Argm::Mismatched_bracket,
                          group_str.substr(0, group_str.length()-1));}
      if (group.elipsis == -1 && !positional.size())
        throw Signal_argm(Argm::Elipsis_first_arg, group.str());
      for (std::vector<std::string>::const_iterator i = group.names.begin();
           i != group.names.end(); ++i) {
        check_for_duplicates(*i);
        if (*i == "--" && group.names.size() > 1)
          throw Signal_argm(Argm::Dash_dash_argument, group.str());}
      if (first_name[0] != '-') positional.push_back(group);
      else if (first_name == "--") explicit_dash_dash = true;
      else if (flags_i == IGNORANT)
        throw Signal_argm(Argm::Ignored_flag, group.str());
      else flag_options[first_name] = group;}}

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
  
void Function::check_for_duplicates(const std::string& name) {
  if (parameter_names.find(name) != parameter_names.end())
    throw Signal_argm(Argm::Duplicate_parameter, name);
  parameter_names.insert(name);}

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
    if (!non_prototype) {
      if (flags == SOME || flag_options.begin() != flag_options.end())
        locals_map.local("-*", "");
      int available = invoking_argm.argc()-1;
      int needed = required_argc;
      Argm::const_iterator i = invoking_argm.begin()+1;
      if (flags != IGNORANT) while (i != invoking_argm.end() && (*i)[0] == '-'){
        std::map<std::string, Parameter_group>::const_iterator j =
                                                         flag_options.find(*i);
        if (j != flag_options.end()) {
          std::string flag = j->second.names[0];
          available -= j->second.names.size();
          for (std::vector<std::string>::size_type k = 0;
               i != invoking_argm.end() && k < j->second.names.size(); ++k) {
            locals_map.local_or_append_word(flag, *i);
            if (k) locals_map.local_or_append_word(j->second.names[k], *i);
            locals_map.append_word_if_exists("-*", *i++);
            if (j->second.elipsis == k) for (;available > needed; --available) {
              locals_map.append_word_locally(flag, *i);
              if (k) locals_map.append_word_locally(j->second.names[k], *i);
              locals_map.append_word_if_exists("-*", *i++);}}}
        else {
          locals_map.append_word_if_exists("-*", *i);
          if(*i == "--") {                                      // "discard" --
            locals_map.local(*i, *i);
            ++i, --available; break;}
          else if (flags == ALL) throw Signal_argm(Argm::Unrecognized_flag, *i);
          else ++i, --available;}}
      std::vector<Parameter_group>::const_iterator j = positional.begin();
      if (available < needed) {
        unsigned non_optional = available + required_argc - needed;
        throw Signal_argm(Argm::Bad_argc, non_optional, required_argc,
                          invoking_argm.argc()-1-non_optional);}
      for (; i != invoking_argm.end() && j != positional.end(); ++j)
        if (j->required || available > needed) {
          if (j->required) --needed;
          else if (available < needed + j->names.size()) {
            unsigned non_optional = available + required_argc - needed;
            throw Signal_argm(Argm::Bad_argc, non_optional, required_argc,
                              invoking_argm.argc()-1-non_optional);}
          available -= j->names.size();
          if (j->elipsis == -1) for (;available > needed; --available)
            locals_map.append_word_locally((j-1)->names.back(), *i++);
          for (std::vector<std::string>::size_type k = 0; k < j->names.size();
               k++){
            locals_map.local(j->names[k], *i++);
            if (j->elipsis == k) for (;available > needed; --available)
              locals_map.append_word_locally(j->names[k], *i++);}}
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

std::string escape(const std::string& src) {
  if (!src.length()) return "()";
  else return src;}

// convert the function to a string. except for the handling of the name this
// is the inverse of the string constructor.
std::string Function::str() const {
  std::string prototype;
  if (non_prototype) {
    if (!is_argfunction_name(name()))
      prototype = ".function " + escape(name()) + " ";}
  else {
    switch (flags) {
      case ALL: prototype = ".function_all_flags " + escape(name()) + " ";break;
      case SOME:
        prototype = ".function_some_flags " + escape(name()) + " "; break;
      case IGNORANT:
        prototype = ".function_flag_ignorant " + escape(name()) + " "; break;}
    for (std::map<std::string, Parameter_group>::const_iterator i =
             flag_options.begin(); i != flag_options.end(); ++i)
      prototype.append(i->second.str() + " ");
    if (explicit_dash_dash) prototype.append("[--] ");
    for (std::vector<Parameter_group>::const_iterator i = positional.begin();
         i != positional.end(); ++i)
      prototype.append(i->str() + " ");}
  return prototype + body.str();}
