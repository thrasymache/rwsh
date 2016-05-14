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

namespace {
void p_elipsis(Variable_map& locals, Argm::const_iterator& f_arg,
               int& available, const std::string& name, const std::string* flag,
               int needed, enum Dash_dash_type dash_dash) {
  locals.local(name, word_from_value(*f_arg));
  if (flag) {
    locals.append_word_if_exists("-*", *f_arg);
    if (*flag != name) locals.append_word_locally(*flag, *f_arg);}
  for (f_arg++; available > needed; --available, f_arg++) {
    if ((*f_arg)[0] == '-' && !dash_dash && f_arg->length() > 1)
      throw Signal_argm(Argm::Flag_in_elipsis, *f_arg);
    locals.append_word_locally(name, *f_arg);
    if (flag) {
      locals.append_word_locally("-*", *f_arg);
      if (*flag != name) locals.append_word_locally(*flag, *f_arg);}}}
} // end unnamed namespace

Parameter_group::Parameter_group(Argm::const_iterator& focus,
                                 Argm::const_iterator end,
                                 std::set<std::string>& parameter_names) :
    elipsis(-2), required((*focus)[0] != '['), names() {
  bool group_end;
  do {
    group_end = (*focus)[focus->length()-1] == ']';
    bool group_begin = (*focus)[0] == '[';
    std::string name(
       focus->substr(group_begin, focus->length() - group_begin - group_end));
    if (name != "...") names.push_back(name);
    else if (!parameter_names.insert(name).second)
      throw Signal_argm(Argm::Duplicate_parameter, name);
    else elipsis = names.size()-1;}
  while (!required && !group_end && ++focus != end);
  if (!required && !group_end) {
    std::string gs(str());
    throw Signal_argm(Argm::Mismatched_bracket, gs.substr(0, gs.length()-1));}
  for (std::vector<std::string>::const_iterator i = names.begin();
       i != names.end(); ++i)
    if (!parameter_names.insert(*i).second)
      throw Signal_argm(Argm::Duplicate_parameter, *i);
    else if (names.size() > 1)
      if (*i == "-*")      throw Signal_argm(Argm::Dash_star_argument, str());
      else if (*i == "--") throw Signal_argm(Argm::Dash_dash_argument, str());}

void Parameter_group::arg_to_param(Variable_map& locals,
                                   int& available, int& needed,
                                   std::string& missing,
                                   Argm::const_iterator& f_arg,
                                   const Argm::const_iterator end,
                                   const std::string* flag,
                                   const std::string& elipsis_var,
                                   enum Dash_dash_type dash_dash) const {
  available -= names.size();
  if (required) --needed;
  if (elipsis == -1) {
    locals.unset(elipsis_var);
    p_elipsis(locals, --f_arg, available, elipsis_var, flag, needed,
              dash_dash);}
  std::vector<std::string>::size_type k = 0;
  for (; f_arg != end && k < names.size(); k++)
    if (elipsis == k) p_elipsis(locals, f_arg, available, elipsis_var,
                                flag, needed, dash_dash);
    else if (flag) {
        locals.append_word_if_exists("-*", *f_arg);
        if (*flag != names[k]) locals.append_word_locally(*flag, *f_arg);
        locals.local_or_append_word(names[k], *f_arg++);}
    else locals.local(names[k], *f_arg++);
  if (f_arg == end) while(k < names.size())
    missing += (missing.length()?" ":"") + names[k++];}

std::string Parameter_group::str() const {
  if (!names.size())
    if (required) return "...";
    else return "[...]";
  else if (required) return names[0] + (elipsis? "": " ...");
  else {
    std::string result("[");
    if (elipsis == -1) result.append("... ");
    for (std::vector<std::string>::size_type j = 0; j < names.size(); ++j)
       result.append((j? " ": "") + names[j] + (elipsis == j? " ...": ""));
    return result + "]";}}

Prototype::Prototype(bool non_prototype_i) :
    positional(), required_argc(), flag_options(), parameter_names(),
    non_prototype(non_prototype_i), flags(ALL), bare_dash_dash(false),
    dash_dash_position(-1), elipsis_var("") {}

Prototype::Prototype(Argm::const_iterator fp, Argm::const_iterator end,
                     bool non_prototype_i, Flag_type flags_i) :
    positional(), required_argc(), flag_options(), parameter_names(),
    non_prototype(non_prototype_i), flags(flags_i), bare_dash_dash(false),
    dash_dash_position(-1), elipsis_var("") {
  bool has_elipsis = false;
  for (; fp != end; ++fp) {
    Parameter_group group(fp, end, parameter_names);
    if (has_elipsis && !group.required)
      throw Signal_argm(Argm::Post_elipsis_option, group.str());
    else if (group.elipsis == -1) {
      if (!positional.size())
        throw Signal_argm(Argm::Elipsis_first_arg, group.str());
      has_elipsis = true;
      elipsis_var = positional.back().names.back();
      if (group.names.size()) positional.push_back(group);
      else if (positional.back().names.size() != 1)
        throw Signal_argm(Argm::Elipsis_out_of_option_group,
                          positional.back().str());
      else positional.back().elipsis = 0;}
    else if (group.names[0] == "--") {
      dash_dash_position = positional.size();
      bare_dash_dash = group.required;
      if (!dash_dash_position && (flag_options.size() || flags == SOME))
        throw Signal_argm(Argm::Ambiguous_prototype_dash_dash, str());}
    else if (group.names[0] == "-*") flags = SOME;
    else if (group.required || group.names[0][0] != '-' ||
             group.names[0].length() == 1) {
      required_argc += group.required;
      positional.push_back(group);}
    else if (dash_dash_position != -1)
      throw Signal_argm(Argm::Post_dash_dash_flag, group.str());
    else flag_options[group.names[0]] = group;
    if (group.elipsis >= 0) {
      has_elipsis = true;
      elipsis_var = group.names[group.elipsis];}}}

Variable_map Prototype::arg_to_param(const Argm& argm) const {
  Variable_map locals(argm.parent_map());
  if (non_prototype) return locals;
  enum Dash_dash_type dash_dash = dash_dash_position? UNSEEN:
                                  bare_dash_dash? BARE: BRACKET;
  if (flags == SOME || flag_options.size()) locals.local("-*", "");
  int needed = required_argc;
  std::string missing;
  Argm::const_iterator f_arg = argm.begin()+1;
  std::vector<Parameter_group>::const_iterator param = positional.begin();
  for (int available = argm.argc()-1; f_arg != argm.end();)
    if ((*f_arg)[0] == '-' && f_arg->length() > 1 && dash_dash != BARE) {
      std::map<std::string, Parameter_group>::const_iterator h =
                                                     flag_options.find(*f_arg);
      if (dash_dash == BRACKET && *f_arg != "--")
        throw Signal_argm(Argm::Tardy_flag, *f_arg);
      else if (h != flag_options.end())
        h->second.arg_to_param(locals, available, needed, missing, f_arg,
                      argm.end(), &h->second.names[0], elipsis_var, dash_dash);
      else {
        if (*f_arg == "--") {
          locals.local("--", "--");
          dash_dash = BARE;}
        else if (flags == ALL)
          throw Signal_argm(Argm::Unrecognized_flag, *f_arg);
        locals.append_word_if_exists("-*", *f_arg++);
        --available;}}
    else if (param == positional.end()) break;
    else {
      if (param->required || available > needed)
        param->arg_to_param(locals, available, needed, missing, f_arg,
                            argm.end(), NULL, elipsis_var, dash_dash);
      if (++param - positional.begin() == dash_dash_position)
        dash_dash = bare_dash_dash? BARE: BRACKET;}
  if (f_arg != argm.end() || needed || missing.length())
    bad_args(missing, locals, f_arg, argm.end(), param);
  else if (param != positional.end() && param->elipsis == -1) {
    const std::string& var((param-1)->names.back());
    locals.set(var, word_from_value(locals.get(var)));}
  return locals;}

Argm Prototype::bad_args(std::string& missing, const Variable_map& locals,
                    Argm::const_iterator f_arg, Argm::const_iterator end,
                    std::vector<Parameter_group>::const_iterator param) const {
  std::string assigned;
  for (Variable_map::const_iterator k =locals.begin(); k!=locals.end(); ++k)
    assigned += (assigned.length()? " (": "(") + k->first + " " +
                 word_from_value(k->second) + ")";
  for (;param != positional.end(); ++param) if (param->required)
    missing += (missing.length()?" ":"") + param->names[0];
  std::string unassigned;
  while (f_arg != end)
    unassigned += (unassigned.length()?" ":"") + *f_arg++;
  throw Signal_argm(Argm::Bad_args, str(), assigned, missing, unassigned);}

std::string escape(const std::string& src) {
  if (!src.length()) return "()";
  else return src;}

std::string Prototype::str() const {
  std::string result;
  if (flags == SOME) result = "[-*] ";
  for (std::map<std::string, Parameter_group>::const_iterator i =
           flag_options.begin(); i != flag_options.end(); ++i)
    result.append(i->second.str() + " ");
  for (int i=0; i <= positional.size(); ++i) {
    if (dash_dash_position == i)
      if (bare_dash_dash) result.append("-- ");
      else result.append("[--] ");
    if (i < positional.size()) result.append(positional[i].str() + " ");}
  return result.substr(0, result.length()-1);}
