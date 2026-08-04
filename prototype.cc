// The definition of the Prototype class which specifies the translation
// between the arguments passed to and the parameters received by a function
// or the argument function of .scope
//
// Copyright (C) 2015-2026 Samuel Newbold
#include <set>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sys/time.h>

#include "argv.h"
#include "rwsh_stream.h"
#include "prototype.h"
#include "variable_map.h"

#include "argm.h"

void Parameter_group::p_elipsis(Parsing_state& state, const std::string* flag)
        const {
  if (flag) {
    state.append_cur_arg("-*", parent.is_reinterpret);
    if (*flag != parent.elipsis_var)
      state.append_cur_arg(*flag, parent.is_reinterpret);}
  for (state.cur_arg++; state.available > state.pos_c;
       --state.available, state.cur_arg++) {
    if (state.is_flag_arg())
      state.add_error(Exception(E::Flag_in_elipsis, *state.cur_arg, str()));
    state.append_cur_arg(parent.elipsis_var, parent.is_reinterpret);
    if (flag) {
      state.append_cur_arg("-*", parent.is_reinterpret);
      if (*flag != parent.elipsis_var)
        state.append_cur_arg(*flag, parent.is_reinterpret);}}}

Parameter_group::Parameter_group(Argv::const_iterator& focus,
                                 Argv::const_iterator end,
                                 std::set<std::string>& parameter_names,
                                 const Prototype& parent_i) :
    elipsis(-2), has_argfunction(false), names(), required((*focus)[0] != '['),
    parent(parent_i) {
  bool group_end;
  do {
    group_end = (*focus)[focus->length()-1] == ']';
    bool group_begin = (*focus)[0] == '[';
    std::string name(
       focus->substr(group_begin, focus->length() - group_begin - group_end));
    if (name[0] != '.') names.push_back(name);
    else if (name == "...")
      if (!parameter_names.insert(name).second)
        throw Exception(E::Duplicate_parameter, name);
      else elipsis = names.size()-1;
    else if (name == ".{argfunction}") {
      has_argfunction = true;
      names.push_back(name);}
    else throw Exception(E::Fixed_argument, name);}
  while (!required && !group_end && ++focus != end);
  if (!required && !group_end) {
    std::string gs(str());
    throw Exception(E::Mismatched_bracket, gs.substr(0, gs.length()-1));}
  for (auto i: names)
    if (!parameter_names.insert(i).second && i != "--")
      throw Exception(E::Duplicate_parameter, i);
    else if (names.size() > 1)
      if (i == "-*" || i == "-?")
        throw Exception(E::Dash_star_argument, str());
      else if (i == "--") throw Exception(E::Dash_dash_argument, str());
      else;
    else;}

void Parameter_group::arg_to_param(Parsing_state& state,
                                   const std::string* flag) const {
  state.available -= names.size();
  if (!flag) state.pos_c -= names.size();
  if (required) --state.needed;
  if (elipsis == -1) {
    state.set_get_word_from_value(parent.elipsis_var);
    --state.cur_arg;
    p_elipsis(state, flag);}
  std::vector<std::string>::difference_type k = 0;
  for (; !state.cur_is_end() && k < names.size(); k++)
    if (elipsis == k) {
      state.locals.param(parent.elipsis_var, word_from_value(*state.cur_arg),
                         parent.is_reinterpret);
      p_elipsis(state, flag);}
    else if (flag) {
        state.append_cur_arg("-*", parent.is_reinterpret);
        if (*flag!=names[k]) state.append_cur_arg(*flag, parent.is_reinterpret);
        state.locals.param_or_append_word(names[k], *state.cur_arg++,
                                          parent.is_reinterpret);}
    else state.locals.param(names[k], *state.cur_arg++, parent.is_reinterpret);
  if (state.cur_is_end())
    while(k < names.size()) {
      state.missing += (state.missing.length()?" ":"") + names[k];
      state.locals.add_undefined(names[k++], parent.is_reinterpret,
                                 parent.is_extra_round);}}

void Parameter_group::add_undefined_params(Variable_map& locals) const {
  for (auto j: names)
    locals.add_undefined(j, parent.is_reinterpret, parent.is_extra_round);}

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
    positional(), required_argc(), pos_argc(),
    exclude_argfunction(true), required_argfunction(false),
    is_reinterpret(false), is_extra_round(false) {}

Prototype::Prototype(const Argv& parameters, bool is_reinterpret_i,
                     bool is_extra_round_i) :
    bare_dash_dash(false), dash_dash_position(-1), elipsis_var(""),
    flag_options(), flags(ALL), parameter_names{"--"},
    positional(), required_argc(), pos_argc(),
    exclude_argfunction(true), required_argfunction(false),
    is_reinterpret(is_reinterpret_i), is_extra_round(is_extra_round_i) {
  bool has_elipsis = false;
  for (auto fp = parameters.begin(); fp != parameters.end(); ++fp) {
    Parameter_group group(fp, parameters.end(), parameter_names, *this);
    if (group.elipsis == -1) {
      if (!positional.size())
        throw Exception(E::Elipsis_first_arg, group.str());
      has_elipsis = true;
      elipsis_var = positional.back().names.back();
      pos_argc += group.names.size();
      if (group.names.size()) positional.push_back(group);
      else positional.back().elipsis = positional.back().names.size()-1;}
    else if (group.names[0] == "--") {
      if (dash_dash_position != -1)
        throw Exception(E::Duplicate_parameter, "--");
      dash_dash_position = positional.size();
      bare_dash_dash = group.required;
      if (!dash_dash_position && (flag_options.size() || flags == SOME))
        throw Exception(E::Ambiguous_prototype_dash_dash, str());}
    else if (group.names[0] == ".{argfunction}") {
      if (group.required) required_argfunction = true;
      exclude_argfunction = false;}
    else if (group.names[0] == "-?" || group.names[0] == "-*") {
      flags = SOME;
      parameter_names.insert({"-*", "-?"});}
    else if (group.required || group.names[0][0] != '-' ||
             group.names[0].length() == 1) {
      required_argc += group.required;
      pos_argc += group.names.size();
      positional.push_back(group);}
    else if (dash_dash_position != -1)
      throw Exception(E::Post_dash_dash_flag, group.str());
    else {
      flag_options.emplace(std::make_pair(group.names[0], group));
      parameter_names.insert("-*");}
    if (group.elipsis >= 0) {
      has_elipsis = true;
      elipsis_var = group.names[group.elipsis];}}}

Parsing_state::Parsing_state(const Argv& argv, int needed_i, int pos_c_i,
		             enum Dash_dash_type dash_dash_i,
		             Variable_map& locals_i, Error_list& exceptions_i) :
    available(argv.size()-1), cur_arg(argv.begin()+1), end(argv.end()),
    needed(needed_i), pos_c(pos_c_i), dash_dash(dash_dash_i), locals(locals_i),
    missing(), exceptions(exceptions_i) {};

void Parsing_state::append_cur_arg(const std::string& key, bool is_reinterpret){
    locals.append_word(key, *cur_arg, is_reinterpret);};

void Parsing_state::add_error(Exception focus) {exceptions.add_error(focus);}

void Parsing_state::set_get_word_from_value(const std::string& key) {
  locals.set(key, word_from_value(locals.get(key)));}

void Prototype::arg_to_param(const Argv& argv, Variable_map& locals,
                             Error_list& exceptions) const {
  enum Dash_dash_type dash_dash = dash_dash_position? UNSEEN:
                                  bare_dash_dash? BARE: BRACKET;
  Parsing_state state(argv, required_argc, pos_argc, dash_dash,
		      locals, exceptions);
  if (state.dash_dash == BRACKET)
    locals.add_undefined("--", is_reinterpret, is_extra_round);
  if (!is_extra_round || !locals.simple_exists("-*"))
    if (flags == SOME)
      locals.param("-*","",is_reinterpret),locals.param("-?","",is_reinterpret);
    else if (flag_options.size()) locals.param("-*", "", is_reinterpret);
  for (auto j: flag_options)
    j.second.add_undefined_params(locals);
  auto param = positional.begin();
  while (!state.cur_is_end())
    if (state.is_flag_arg()) {
      auto h = flag_options.find(*state.cur_arg);
      if (state.dash_dash == BRACKET && *state.cur_arg != "--") {
        --state.available;
        state.add_error(Exception(E::Tardy_flag, *state.cur_arg++));}
      else if (h != flag_options.end())
        h->second.arg_to_param(state, &h->second.names[0]);
      else {
        if (*state.cur_arg == "--") {
          state.locals.param("--", "--", is_reinterpret);
          state.dash_dash = BARE;}
        else if (flags == ALL)
          state.add_error(Exception(E::Unrecognized_flag,*state.cur_arg,str()));
        if (flags == SOME) state.append_cur_arg("-?", is_reinterpret);
        if (flag_options.size() || flags == SOME)
          state.append_cur_arg("-*", is_reinterpret);
        ++state.cur_arg, --state.available;}}
    else if (param == positional.end()) break;
    else {
      if (param->required || state.available > state.needed)
        param->arg_to_param(state, nullptr);
      else param->add_undefined_params(state.locals);
      if (++param - positional.begin() == dash_dash_position)
        state.dash_dash = bare_dash_dash? BARE: BRACKET;}
  if (param != positional.end()) {
    if (param->elipsis == -1) {
      const std::string& var((param-1)->names.back());
      if (!state.locals.simple_exists(var))
	state.locals.add_undefined(var, is_reinterpret, is_extra_round);
      else state.set_get_word_from_value(var);}
    while (param != positional.end()) {
      if (param->required)
        state.missing += (state.missing.length()?" ":"") + param->names[0];
      param++->add_undefined_params(state.locals);}}
  if (state.excess_or_missing_args()) bad_args(state);
  if (state.exceptions.size()) state.locals.bless_unused_vars_without_usage();}

void Prototype::bad_args(Parsing_state& state) const {
  std::string assigned;
  for (auto k: parameter_names) if (state.locals.exists_without_check(k))
    assigned += (assigned.length()? " (": "(") + k + " " +
                 word_from_value(state.locals.get(k)) + ")";
  std::string unassigned;
  while (!state.cur_is_end())
    unassigned += (unassigned.length()?" ":"") + *state.cur_arg++;
  state.add_error(Exception(E::Bad_args, str(), assigned, state.missing,
                            unassigned));}

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
                unused_flag? E::Unused_variable: E::Unchecked_variable,
                names[0]));}
  else if (vars->exists_without_check(names[0])) {
    for (auto j=names.begin()+1; j != names.end(); ++j)
      if (!vars->used_vars_contains(*j)) {
        unused_flag = true;
        errors.add_error(Exception(E::Unused_variable, *j));
        vars->used_vars_insert(*j);}
    if (unused_flag)
      errors.add_error(Exception(E::Unused_variable, names[0]));}
  else if (!vars->locals_listed) {
    bool checked = false;
    for (auto j: names) if (vars->checked_vars_contains(j)) checked = true;
    if (!checked) {
      for (auto j: names)
        errors.add_error(Exception(E::Unchecked_variable, j));
      return true;}}
  vars->used_vars_insert(names[0]);
  return unused_flag;}

void Parameter_group::unused_pos_var_check(Variable_map* vars,
                                           Error_list& errors) const {
  if (vars->exists_without_check(names[0]))
    for (auto j: names)
      if (vars->exists_without_check(j) && !vars->used_vars_contains(j)) {
        errors.add_error(Exception(E::Unused_variable, j));
        vars->used_vars_insert(j);}
      else;
  else if (!vars->locals_listed) {
    bool checked = false;
    for (auto j: names) if (vars->checked_vars_contains(j)) checked = true;
    if (!checked) for (auto j: names)
      errors.add_error(Exception(E::Unchecked_variable, j));}}

void Prototype::unused_var_check(Variable_map* vars, Error_list& errors) const {
  bool unused_flag = false;
  if (vars->used_vars_contains("-*")) for (auto i: flag_options)
    i.second.bless_unused_vars(vars);
  else {
    for (auto i: flag_options)
      unused_flag |= i.second.unused_flag_var_check(vars, errors);
    if (flags == SOME && !vars->used_vars_contains("-?")) {
      unused_flag = true;
      errors.add_error(Exception(E::Unused_variable, "-?"));}}
  if (unused_flag) errors.add_error(Exception(E::Unused_variable, "-*"));
  vars->used_vars_insert("-*");  // in the absent else case $-* is not defined
  vars->used_vars_insert("-?");  // in the absent else case $-? is not defined
  for (auto i: positional) i.unused_pos_var_check(vars, errors);
  for (auto j: vars->locals())
    if (!vars->used_vars_contains(j) && !vars->undefined_vars_contains(j)) {
      errors.add_error(Exception(E::Unused_variable, j));
      vars->used_vars_insert(j);}}
