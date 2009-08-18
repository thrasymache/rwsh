// The definition of the Argv_t class, which contains the arguments that may
// be passed to an executable.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <map>
#include <sstream>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "function.h"
#include "variable_map.h"

Argv_t::Argv_t(void) : argfunction_v(0), 
  input(default_input), output(default_output), error(default_error) {}

Argv_t::Argv_t(const Argv_t& src) : Base(src), 
  argfunction_v(src.argfunction()->copy_pointer()), 
  input(default_input), output(default_output), error(default_error) {}

Argv_t& Argv_t::operator=(const Argv_t& src) {
  Base::clear(); 
  std::copy(src.begin(), src.end(), std::back_inserter(*this));
  delete argfunction_v;
  argfunction_v = src.argfunction()->copy_pointer();
  input = src.input;
  output = src.output;
  error = src.error;}

Argv_t::~Argv_t(void) {
  delete argfunction_v;}

// convert to a string. inverse of constructor.
std::string Argv_t::str(void) const {
  std::string result;
  for (const_iterator i=begin(); i != end()-1; ++i) result += *i + ' ';
  result += back();
  if (!input.is_default()) result += " " + input.str();
  if (!output.is_default()) result += " " + output.str();
  if (!error.is_default()) result += " " + error.str();
  if (argfunction()) result += " " + argfunction()->str();
  return result;}

void Argv_t::set_argfunction(Function_t* val) {argfunction_v = val;};

void Argv_t::append_to_errno(const std::string& value) const {
  var_map->append_to_errno(value);}

// returns variables that are defined in the argument vector other than $* 
// (i.e. positional parameters and $#)
std::string Argv_t::get_var(const std::string& key) const {
  switch (key[0]) {
    case '#': {
      std::ostringstream str;
      str <<size();
      return str.str();}
    case '1': case '2': case '3': case '4': case '5': case '6': case '7':
              case '8': case '9': case '0': {
      int n = std::atoi(key.c_str());
      if (size() > n) return (*this)[n];
      else return std::string();} //{
        //Executable_t::caught_signal = Executable_t::SIGVAR;
        //Executable_t::call_stack.push_back(key);
        //throw Undefined_variable_t(key);}}
    default: return var_map->get(key);}}

int Argv_t::set_var(const std::string& key, const std::string& value) const {
  switch (key[0]) {
    case '#': case '1': case '2': case '3': case '4': case '5': case '6': 
              case '7': case '8': case '9': case '0': return 2;
    default: return var_map->set(key, value);}}

bool Argv_t::var_exists(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': return true; 
    case '1': case '2': case '3': case '4': case '5': case '6': 
              case '7': case '8': case '9': case '0': {
      int n = std::atoi(key.c_str());
      return size() > n;}
    default: return var_map->exists(key);}}

int Argv_t::global_var(const std::string& key, 
                        const std::string& value) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5': 
              case '6': case '7': case '8': case '9': case '0': return 2;
    default: return var_map->add(key, value);}}

int Argv_t::unset_var(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5': 
              case '6': case '7': case '8': case '9': case '0': return 3;
    default: return var_map->unset(key);}}

unsigned Argv_t::max_nesting(void) const {return var_map->max_nesting();}

char** Argv_t::export_env(void) const {return var_map->export_env();}

// algorithm that is the meat of Old_argv constructor
template<class In>char** copy_to_cstr(In first, In last, char** res) {
  while (first != last) {
    *res = new char[(*first).length()+1];
    strcpy(*res++, (*first++).c_str());}
  *res = 0;
  return res;}

Argument_count_t::Argument_count_t(unsigned given, unsigned expected) {
  push_back("rwsh.argument_count");
  /*push_back(given); push_back(expected);*/}

Missing_argfunction_t::Missing_argfunction_t() {
  push_back("rwsh.missing_argfunction");}

Arguments_to_argfunction_t::Arguments_to_argfunction_t(
      const std::string& argfunction_type) {
  push_back("rwsh.arguments_for_argfunction");
  push_back(argfunction_type);}

Bad_argfunction_style_t::Bad_argfunction_style_t(
      const std::string& argfunction_style) {
  push_back("rwsh.bad_argfunction_style");
  push_back(argfunction_style);}

Bad_if_nest_t::Bad_if_nest_t() {push_back("rwsh.bad_if_nest");}

Divide_by_zero_t::Divide_by_zero_t(const std::string& value) {
  push_back("rwsh.divide_by_zero");
  push_back(value);}

Double_redirection_t::Double_redirection_t(const std::string& first, 
      const std::string& second) {
  push_back("rwsh.double_redirection");
  push_back(first);
  push_back(second);}

Else_without_if_t::Else_without_if_t() {push_back("rwsh.else_without_if");}

Failed_substitution_t::Failed_substitution_t(const std::string& function) {
  push_back("rwsh.failed_substitution");
  push_back(function);}

File_open_failure_t::File_open_failure_t(const std::string& file_name) {
  push_back("rwsh.file_open_failure");
  push_back(file_name);}

If_before_else_t::If_before_else_t() {push_back("rwsh.if_before_else");}

Invalid_word_selection_t::Invalid_word_selection_t(
      const std::string& selection) {
  push_back("rwsh.invalid_word_selection");
  push_back(selection);}

Input_range_t::Input_range_t(const std::string& value) {
  push_back("rwsh.input_range");
  push_back(value);}

Line_continuation_t::Line_continuation_t() {
  push_back("rwsh.line_continuation");}

Mismatched_brace_t::Mismatched_brace_t(const std::string& prefix) {
  push_back("rwsh.mismatched_brace");
  push_back(prefix);}

Mismatched_parenthesis_t::Mismatched_parenthesis_t(const std::string& prefix) {
  push_back("rwsh.mismatched_parenthesis");
  push_back(prefix);}

Multiple_argfunctions_t::Multiple_argfunctions_t() {
  push_back("rwsh.multiple_argfunctions");}

Not_soon_enough_t::Not_soon_enough_t(const std::string& argument) : 
      Argv_t() {
  push_back("rwsh.not_soon_enough");
  push_back(argument);}

Not_executable_t::Not_executable_t(const std::string& file_name) {
  push_back("rwsh.not_executable");
  push_back(file_name);}

Not_a_number_t::Not_a_number_t(const std::string& value) {
  push_back("rwsh.not_a_number");
  push_back(value);}

Result_range_t::Result_range_t(const std::string& lhs, const std::string& rhs) {
  push_back("rwsh.result_range");
  push_back(lhs);
  push_back(rhs);}

Unclosed_brace_t::Unclosed_brace_t(const std::string& prefix) {
  push_back("rwsh.subprompt");
  push_back(prefix);}

Unclosed_parenthesis_t::Unclosed_parenthesis_t(const std::string& prefix) {
  push_back("rwsh.subprompt");
  push_back(prefix);}

Undefined_variable_t::Undefined_variable_t(const std::string& variable) {
  push_back("rwsh.undefined_variable");
  push_back(variable);}
  
Old_argv_t::Old_argv_t(const Argv_t& src) : argc_v(src.size()) {
  focus = new char*[src.size()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv_t::~Old_argv_t(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}

