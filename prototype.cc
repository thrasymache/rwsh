// The definition of the Prototype class which specifies the translation
// between the arguments passed to and the parameters received by a function
// or the argument function of .scope
//
// Copyright (C) 2015-2023 Samuel Newbold
#include <set>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sys/time.h>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "executable.h"
#include "prototype.h"

void Parameter_group::p_elipsis(Variable_map& locals,
        Argm::const_iterator& f_arg, int& available, const std::string& name,
        const std::string* flag, int needed, enum Dash_dash_type dash_dash,
        bool is_reassign, Error_list& exceptions) const {
  if (flag) {
    locals.append_word("-*", *f_arg, is_reassign);
    if (*flag != name) locals.append_word(*flag, *f_arg, is_reassign);}
  for (f_arg++; available > needed; --available, f_arg++) {
    if ((*f_arg)[0] == '-' && !dash_dash && f_arg->length() > 1)
      exceptions.add_error(Exception(Argm::Flag_in_elipsis, *f_arg, str()));
    locals.append_word(name, *f_arg, is_reassign);
    if (flag) {
      locals.append_word("-*", *f_arg, is_reassign);
      if (*flag != name) locals.append_word(*flag, *f_arg, is_reassign);}}}

Parameter_group::Parameter_group(Argm::const_iterator& focus,
                                 Argm::const_iterator end,
                                 std::set<std::string>& parameter_names) :
    elipsis(-2), has_argfunction(false), names(), required((*focus)[0] != '[') {
  bool group_end;
  do {
    group_end = (*focus)[focus->length()-1] == ']';
    bool group_begin = (*focus)[0] == '[';
    std::string name(
       focus->substr(group_begin, focus->length() - group_begin - group_end));
    if (name[0] != '.') names.push_back(name);
    else if (name == "...")
      if (!parameter_names.insert(name).second)
        throw Exception(Argm::Duplicate_parameter, name);
      else elipsis = names.size()-1;
    else if (name == ".{argfunction}") {
      has_argfunction = true;
      names.push_back(name);}
    else throw Exception(Argm::Fixed_argument, name);}
  while (!required && !group_end && ++focus != end);
  if (!required && !group_end) {
    std::string gs(str());
    throw Exception(Argm::Mismatched_bracket, gs.substr(0, gs.length()-1));}
  for (auto i: names)
    if (!parameter_names.insert(i).second && i != "--")
      throw Exception(Argm::Duplicate_parameter, i);
    else if (names.size() > 1)
      if (i == "-*" || i == "-?")
        throw Exception(Argm::Dash_star_argument, str());
      else if (i == "--") throw Exception(Argm::Dash_dash_argument, str());
      else;
    else;}

void Parameter_group::arg_to_param(int& available, int& needed,
                                   std::string& missing,
                                   Argm::const_iterator& f_arg,
                                   const Argm::const_iterator end,
                                   const std::string* flag,
                                   const std::string& elipsis_var,
                                   enum Dash_dash_type dash_dash,
                                   bool is_reassign,
                                   Variable_map& locals,
                                   Error_list& exceptions) const {
  available -= names.size();
  if (required) --needed;
  if (elipsis == -1) {
    locals.set(elipsis_var, word_from_value(locals.get(elipsis_var)));
    p_elipsis(locals, --f_arg, available, elipsis_var, flag, needed,
              dash_dash, is_reassign, exceptions);}
  std::vector<std::string>::difference_type k = 0;
  for (; f_arg != end &&
      k < (std::vector<std::string>::difference_type) names.size(); k++)
    if (elipsis == k) {
      locals.param(elipsis_var, word_from_value(*f_arg), is_reassign);
      p_elipsis(locals, f_arg, available, elipsis_var, flag, needed, dash_dash,
                is_reassign, exceptions);}
    else if (flag) {
        locals.append_word("-*", *f_arg, is_reassign);
        if (*flag != names[k]) locals.append_word(*flag, *f_arg, is_reassign);
        locals.param_or_append_word(names[k], *f_arg++, is_reassign);}
    else locals.param(names[k], *f_arg++, is_reassign);
  if (f_arg == end)
    while(k < (std::vector<std::string>::difference_type) names.size()) {
      missing += (missing.length()?" ":"") + names[k];
      locals.add_undefined(names[k++], is_reassign);}}

void Parameter_group::add_undefined_params(Variable_map& locals,
                                           bool is_reassign) const {
  for (auto j: names) locals.add_undefined(j, is_reassign);}

std::string Parameter_group::str() const {
  if (!names.size())
    if (required) return "...";
    else return "[...]";
  else if (required) return names[0] + (elipsis? "": " ...");
  else {
    std::string result("[");
    if (elipsis == -1) result.append("... ");
    for (std::vector<std::string>::difference_type j = 0;
         j < (std::vector<std::string>::difference_type) names.size(); ++j)
       result.append((j? " ": "") + names[j] + (elipsis == j? " ...": ""));
    return result + "]";}}

Prototype::Prototype(void) :
    bare_dash_dash(false), dash_dash_position(-1), elipsis_var(""),
    flag_options(), flags(ALL), parameter_names{"--"},
    positional(), required_argc(),
    exclude_argfunction(true), required_argfunction(false) {}

Prototype::Prototype(const Argv& parameters) :
    bare_dash_dash(false), dash_dash_position(-1), elipsis_var(""),
    flag_options(), flags(ALL), parameter_names{"--"},
    positional(), required_argc(),
    exclude_argfunction(true), required_argfunction(false) {
  bool has_elipsis = false;
  for (auto fp = parameters.begin(); fp != parameters.end(); ++fp) {
    Parameter_group group(fp, parameters.end(), parameter_names);
    if (has_elipsis && !group.required && !group.has_argfunction)
      throw Exception(Argm::Post_elipsis_option, group.str());
    else if (group.elipsis == -1) {
      if (!positional.size())
        throw Exception(Argm::Elipsis_first_arg, group.str());
      has_elipsis = true;
      elipsis_var = positional.back().names.back();
      if (group.names.size()) positional.push_back(group);
      else if (positional.back().names.size() != 1)
        throw Exception(Argm::Elipsis_out_of_option_group,
                        positional.back().str());
      else positional.back().elipsis = 0;}
    else if (group.names[0] == "--") {
      if (dash_dash_position != -1)
        throw Exception(Argm::Duplicate_parameter, "--");
      dash_dash_position = positional.size();
      bare_dash_dash = group.required;
      if (!dash_dash_position && (flag_options.size() || flags == SOME))
        throw Exception(Argm::Ambiguous_prototype_dash_dash, str());}
    else if (group.names[0] == ".{argfunction}") {
      if (group.required) required_argfunction = true;
      exclude_argfunction = false;}
    else if (group.names[0] == "-?" || group.names[0] == "-*") {
      flags = SOME;
      parameter_names.insert({"-*", "-?"});}
    else if (group.required || group.names[0][0] != '-' ||
             group.names[0].length() == 1) {
      required_argc += group.required;
      positional.push_back(group);}
    else if (dash_dash_position != -1)
      throw Exception(Argm::Post_dash_dash_flag, group.str());
    else {
      flag_options[group.names[0]] = group;
      parameter_names.insert("-*");}
    if (group.elipsis >= 0) {
      has_elipsis = true;
      elipsis_var = group.names[group.elipsis];}}}

void Prototype::arg_to_param(const Argv& argv, Variable_map& locals,
                             Error_list& exceptions) const {
  arg_to_param_internal(argv, false, locals, exceptions);}

void Prototype::arg_to_param_internal(const Argv& argv, bool is_reassign,
                        Variable_map& locals, Error_list& exceptions) const {
  enum Dash_dash_type dash_dash = dash_dash_position? UNSEEN:
                                  bare_dash_dash? BARE: BRACKET;
  if (dash_dash == BRACKET) locals.add_undefined("--", is_reassign);
  if (flags == SOME)
    locals.param("-*","", is_reassign), locals.param("-?","", is_reassign);
  else if (flag_options.size()) locals.param("-*", "", is_reassign);
  for (auto j: flag_options)
    j.second.add_undefined_params(locals, is_reassign);
  int needed = required_argc;
  std::string missing;
  auto f_arg = argv.begin()+1;
  auto param = positional.begin();
  for (int available = argv.size()-1; f_arg != argv.end();)
    if ((*f_arg)[0] == '-' && f_arg->length() > 1 && dash_dash != BARE) {
      auto h = flag_options.find(*f_arg);
      if (dash_dash == BRACKET && *f_arg != "--") {
        --available;
        exceptions.add_error(Exception(Argm::Tardy_flag, *f_arg++));}
      else if (h != flag_options.end())
        h->second.arg_to_param(available, needed, missing, f_arg, argv.end(),
                               &h->second.names[0], elipsis_var, dash_dash,
                               is_reassign, locals, exceptions);
      else {
        if (*f_arg == "--") {
          locals.param("--", "--", is_reassign);
          dash_dash = BARE;}
        else if (flags == ALL)
          exceptions.add_error(Exception(Argm::Unrecognized_flag, *f_arg, str()));
        if (flags == SOME) locals.append_word("-?", *f_arg, is_reassign);
        if (flag_options.size() || flags == SOME)
          locals.append_word("-*", *f_arg, is_reassign);
        ++f_arg, --available;}}
    else if (param == positional.end()) break;
    else {
      if (param->required || available > needed)
        param->arg_to_param(available, needed, missing, f_arg, argv.end(),
                            nullptr, elipsis_var, dash_dash, is_reassign,
                            locals, exceptions);
      else param->add_undefined_params(locals, is_reassign);
      if (++param - positional.begin() == dash_dash_position)
        dash_dash = bare_dash_dash? BARE: BRACKET;}
  if (param != positional.end()) {
    if (param->elipsis == -1) {
      const std::string& var((param-1)->names.back());
      if (!locals.simple_exists(var)) locals.add_undefined(var, is_reassign);
      else locals.set(var, word_from_value(locals.get(var)));}
    while (param != positional.end()) {
      if (param->required)
        missing += (missing.length()?" ":"") + param->names[0];
      param++->add_undefined_params(locals, is_reassign);}}
  if (f_arg != argv.end() || needed || missing.length())
    bad_args(missing, locals, f_arg, argv.end(), exceptions);
  if (exceptions.size()) locals.bless_unused_vars_without_usage();}

void Prototype::bad_args(std::string& missing, Variable_map& locals,
                    Argm::const_iterator f_arg, Argm::const_iterator end,
                    Error_list& exceptions) const {
  std::string assigned;
  for (auto k: parameter_names) if (locals.exists_without_check(k))
    assigned += (assigned.length()? " (": "(") + k + " " +
                 word_from_value(locals.get(k)) + ")";
  std::string unassigned;
  while (f_arg != end) unassigned += (unassigned.length()?" ":"") + *f_arg++;
  exceptions.add_error(Exception(Argm::Bad_args, str(), assigned, missing,
                                 unassigned));}

void Prototype::reassign(const Argv& argv,
                        Variable_map& locals, Error_list& exceptions) const {
  arg_to_param_internal(argv, true, locals, exceptions);}

std::string Prototype::str() const {
  std::string result;
  if (flags == SOME) result = "[-?] ";
  for (auto i: flag_options)
    result.append(i.second.str() + " ");
  for (int i=0; i <= (int) positional.size(); ++i) {
    if (dash_dash_position == i)
      if (bare_dash_dash) result.append("-- ");
      else result.append("[--] ");
    else;
    if (i < (int)positional.size()) result.append(positional[i].str() + " ");}
  if (required_argfunction) result.append(".{argfunction} ");
  else if (!exclude_argfunction) result.append("[.{argfunction}] ");
  return result.substr(0, result.length()-1);}

void Parameter_group::bless_unused_vars(Variable_map* vars) const {
  // this is not thoroughly tested in the test suite
  for (auto j: names) vars->used_vars_insert(j);}

char Parameter_group::unused_flag_var_check(Variable_map* vars,
                                            Error_list& errors) const {
  bool unused_flag = false;
  if (vars->used_vars_contains(names[0])) bless_unused_vars(vars);
  else if (names.size() == 1)
    if (vars->checked_vars_contains(names[0]) || vars->locals_listed);
    else {
      unused_flag = vars->exists_without_check(names[0]);
      errors.add_error(Exception(
                unused_flag? Argm::Unused_variable: Argm::Unchecked_variable,
                names[0]));}
  else if (vars->exists_without_check(names[0])) {
    for (auto j=names.begin()+1; j != names.end(); ++j)
      if (!vars->used_vars_contains(*j)) {
        unused_flag = true;
        errors.add_error(Exception(Argm::Unused_variable, *j));
        vars->used_vars_insert(*j);}
    if (unused_flag)
      errors.add_error(Exception(Argm::Unused_variable, names[0]));}
  else if (!vars->locals_listed) {
    bool checked = false;
    for (auto j: names) if (vars->checked_vars_contains(j)) checked = true;
    if (!checked) {
      for (auto j: names)
        errors.add_error(Exception(Argm::Unchecked_variable, j));
      return true;}}
  vars->used_vars_insert(names[0]);
  return unused_flag;}

void Parameter_group::unused_pos_var_check(Variable_map* vars,
                                           Error_list& errors) const {
  if (vars->exists_without_check(names[0]))
    for (auto j: names)
      if (vars->exists_without_check(j) && !vars->used_vars_contains(j)) {
        errors.add_error(Exception(Argm::Unused_variable, j));
        vars->used_vars_insert(j);}
      else;
  else if (!vars->locals_listed) {
    bool checked = false;
    for (auto j: names) if (vars->checked_vars_contains(j)) checked = true;
    if (!checked) for (auto j: names)
      errors.add_error(Exception(Argm::Unchecked_variable, j));}}

void Prototype::unused_var_check(Variable_map* vars, Error_list& errors) const {
  if (!vars->usage_checked) vars->usage_checked = true;
  else errors.add_error(Exception(Argm::Internal_error,
                        "variable map usage checked multiple times"));
  bool unused_flag = false;
  if (vars->used_vars_contains("-*")) for (auto i: flag_options)
    i.second.bless_unused_vars(vars);
  else {
    for (auto i: flag_options)
      unused_flag |= i.second.unused_flag_var_check(vars, errors);
    if (flags == SOME && !vars->used_vars_contains("-?")) {
      unused_flag = true;
      errors.add_error(Exception(Argm::Unused_variable, "-?"));}}
  if (unused_flag) errors.add_error(Exception(Argm::Unused_variable, "-*"));
  vars->used_vars_insert("-*");  // in the absent else case $-* is not defined
  vars->used_vars_insert("-?");  // in the absent else case $-? is not defined
  for (auto i: positional) i.unused_pos_var_check(vars, errors);
  for (auto j: vars->locals())
    if (!vars->used_vars_contains(j) && !vars->undefined_vars_contains(j)) {
      errors.add_error(Exception(Argm::Unused_variable, j));
      vars->used_vars_insert(j);}}
