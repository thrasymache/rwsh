// The definition of the Prototype class which specifies the translation
// between the arguments passed to and the parameters received by a function
// or the argument function of .scope
//
// Copyright (C) 2015-2016 Samuel Newbold
#include <set>
#include <string>
#include <vector>
#include <map>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "prototype.h"

Prototype::Prototype(bool non_prototype_i) :
    positional(), required_argc(), flag_options(), parameter_names(),
    non_prototype(non_prototype_i), flags(ALL), explicit_dash_dash(false) {}

Prototype::Prototype(Argm::const_iterator first_parameter,
            Argm::const_iterator parameter_end,
            bool non_prototype_i, Flag_type flags_i) :
    positional(), required_argc(), flag_options(), parameter_names(),
    non_prototype(non_prototype_i), flags(flags_i), explicit_dash_dash(false) {
  bool has_elipsis = false;
  for (Argm::const_iterator i = first_parameter; i != parameter_end; ++i)
    if (*i == "..." || *i == "[...]")
      if (!positional.size()) throw Signal_argm(Argm::Elipsis_first_arg, *i);
      else if (has_elipsis) throw Signal_argm(Argm::Post_elipsis_option, *i);
      else positional.back().elipsis = 0, has_elipsis = true;
    else if (*i == "-*") flags = SOME;
    else if ((*i)[0] != '[')
      if (*i == "--") throw Signal_argm(Argm::Duplicate_parameter, *i);
      else {
        if (parameter_names.find(*i) != parameter_names.end())
          throw Signal_argm(Argm::Duplicate_parameter, *i);
        parameter_names.insert(*i);
        positional.push_back(Parameter_group(true));
        positional.back().names.push_back(*i);
        ++required_argc;}
    else {
      if (has_elipsis) throw Signal_argm(Argm::Post_elipsis_option, *i);
      bool group_end((*i)[i->length()-1] == ']');
      std::string first_name(i->substr(1, i->length() - 1 - group_end));
      Parameter_group group(false);
      if (first_name != "...") group.names.push_back(first_name);
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
      if (first_name == "...") {
        group.elipsis = -1, has_elipsis = true;
        if (!positional.size())
          throw Signal_argm(Argm::Elipsis_first_arg, group.str());}
      for (std::vector<std::string>::const_iterator i = group.names.begin();
           i != group.names.end(); ++i) {
        if (parameter_names.find(*i) != parameter_names.end())
          throw Signal_argm(Argm::Duplicate_parameter, *i);
        parameter_names.insert(*i);
        if (*i == "--" && group.names.size() > 1)
          throw Signal_argm(Argm::Dash_dash_argument, group.str());}
      if (first_name[0] != '-') positional.push_back(group);
      else if (first_name == "--") explicit_dash_dash = true;
      else if (first_name == "-*")
        if (group.names.size() == 1) flags = SOME;
        else throw Signal_argm(Argm::Dash_star_argument, group.str());
      else if (flags == IGNORANT)
        throw Signal_argm(Argm::Ignored_flag, group.str());
      else flag_options[first_name] = group;}}

Variable_map Prototype::arg_to_param(const Argm& argm) const {
  Variable_map locals(argm.parent_map());
  if (non_prototype) return locals;
  if (flags == SOME || flag_options.begin() != flag_options.end())
    locals.local("-*", "");
  int available = argm.argc()-1;
  int needed = required_argc;
  Argm::const_iterator i = argm.begin()+1;
  if (flags != IGNORANT) while (i != argm.end() && (*i)[0] == '-') {
    std::map<std::string, Parameter_group>::const_iterator j =
                                                     flag_options.find(*i);
    if (j != flag_options.end()) {
      std::string flag = j->second.names[0];
      available -= j->second.names.size();
      for (std::vector<std::string>::size_type k = 0;
           i != argm.end() && k < j->second.names.size(); ++k) {
        locals.local_or_append_word(flag, *i);
        if (k) locals.local_or_append_word(j->second.names[k], *i);
        locals.append_word_if_exists("-*", *i++);
        if (j->second.elipsis == k) for (;available > needed; --available) {
          locals.append_word_locally(flag, *i);
          if (k) locals.append_word_locally(j->second.names[k], *i);
          locals.append_word_if_exists("-*", *i++);}}}
    else {
      locals.append_word_if_exists("-*", *i);
      if(*i == "--") {                                      // "discard" --
        locals.local(*i, *i);
        ++i, --available; break;}
      else if (flags == ALL) throw Signal_argm(Argm::Unrecognized_flag, *i);
      else ++i, --available;}}
  std::vector<Parameter_group>::const_iterator j = positional.begin();
  if (available < needed) {
    unsigned non_optional = available + required_argc - needed;
    throw Signal_argm(Argm::Bad_argc, non_optional, required_argc,
                      argm.argc()-1-non_optional);}
  for (; i != argm.end() && j != positional.end(); ++j)
    if (j->required || available > needed) {
      if (j->required) --needed;
      else if (available < needed + j->names.size()) {
        unsigned non_optional = available + required_argc - needed;
        throw Signal_argm(Argm::Bad_argc, non_optional, required_argc,
                          argm.argc()-1-non_optional);}
      available -= j->names.size();
      if (j->elipsis == -1) for (;available > needed; --available)
        locals.append_word_locally((j-1)->names.back(), *i++);
      for (std::vector<std::string>::size_type k = 0; k < j->names.size(); k++)
        if (j->elipsis == k) for (++available; available > needed; --available)
          locals.local_or_append_word(j->names[k], *i++);
        else if (j+1 != positional.end() && (j+1)->elipsis == -1 &&
                 k+1 == j->names.size())
          locals.local_or_append_word(j->names[k], *i++);
        else locals.local(j->names[k], *i++);}
  if (i != argm.end()) {
    unsigned non_optional = required_argc;
    while (i != argm.end()) ++non_optional, ++i;
    throw Signal_argm(Argm::Bad_argc, required_argc+1, required_argc,
                      argm.argc()-1-non_optional);}
  return locals;}

std::string escape(const std::string& src) {
  if (!src.length()) return "()";
  else return src;}

std::string Prototype::str(const std::string &name) const {
  std::string result;
  if (non_prototype) {
    if (!is_argfunction_name(name))
      result = ".function " + escape(name) + " ";}
  else {
    switch (flags) {
      case ALL: result = ".function_all_flags " + escape(name) + " ";break;
      case SOME:
        result = ".function_all_flags " + escape(name) + " [-*] "; break;
      case IGNORANT:
        result = ".function_flag_ignorant " + escape(name) + " "; break;}
    for (std::map<std::string, Parameter_group>::const_iterator i =
             flag_options.begin(); i != flag_options.end(); ++i)
      result.append(i->second.str() + " ");
    if (explicit_dash_dash) result.append("[--] ");
    for (std::vector<Parameter_group>::const_iterator i = positional.begin();
         i != positional.end(); ++i)
      result.append(i->str() + " ");}
  return result;}
