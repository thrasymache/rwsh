// The definition of the Argv class, which contains the arguments that may
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

Argv::Argv(void) : argfunction_v(0), 
  input(default_input), output(default_output), error(default_error) {}

Argv::Argv(const Argv& src) : Base(src), 
  argfunction_v(src.argfunction()->copy_pointer()), 
  input(default_input), output(default_output), error(default_error) {}

Argv& Argv::operator=(const Argv& src) {
  Base::clear(); 
  std::copy(src.begin(), src.end(), std::back_inserter(*this));
  delete argfunction_v;
  argfunction_v = src.argfunction()->copy_pointer();
  input = src.input;
  output = src.output;
  error = src.error;}

Argv::~Argv(void) {
  delete argfunction_v;}

// convert to a string. inverse of constructor.
std::string Argv::str(void) const {
  std::string result;
  for (const_iterator i=begin(); i != end()-1; ++i) result += *i + ' ';
  result += back();
  if (!input.is_default()) result += " " + input.str();
  if (!output.is_default()) result += " " + output.str();
  if (!error.is_default()) result += " " + error.str();
  if (argfunction()) result += " " + argfunction()->str();
  return result;}

void Argv::set_argfunction(Function* val) {argfunction_v = val;};

// returns variables that are defined in the argument vector other than $* 
// (i.e. positional parameters and $#)
std::string Argv::get_var(const std::string& key) const {
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
        //Executable::caught_signal = Executable::SIGVAR;
        //Executable::call_stack.push_back(key);
        //throw Undefined_variable(key);}}
    default: return var_map->get(key);}}

int Argv::set_var(const std::string& key, const std::string& value) const {
  switch (key[0]) {
    case '#': case '1': case '2': case '3': case '4': case '5': case '6': 
              case '7': case '8': case '9': case '0': return 2;
    default: return var_map->set(key, value);}}

bool Argv::var_exists(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': return true; 
    case '1': case '2': case '3': case '4': case '5': case '6': 
              case '7': case '8': case '9': case '0': {
      int n = std::atoi(key.c_str());
      return size() > n;}
    default: return var_map->exists(key);}}

int Argv::global_var(const std::string& key, 
                        const std::string& value) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5': 
              case '6': case '7': case '8': case '9': case '0': return 2;
    default: return var_map->add(key, value);}}

int Argv::unset_var(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5': 
              case '6': case '7': case '8': case '9': case '0': return 3;
    default: return var_map->unset(key);}}

unsigned Argv::max_nesting(void) const {return var_map->max_nesting();}

char** Argv::export_env(void) const {return var_map->export_env();}

// algorithm that is the meat of Old_argv constructor
template<class In>char** copy_to_cstr(In first, In last, char** res) {
  while (first != last) {
    *res = new char[(*first).length()+1];
    strcpy(*res++, (*first++).c_str());}
  *res = 0;
  return res;}

Argument_count::Argument_count(unsigned given, unsigned expected) {
  push_back("rwsh.argument_count");
  /*push_back(given); push_back(expected);*/}

Missing_argfunction::Missing_argfunction() {
  push_back("rwsh.missing_argfunction");}

Arguments_to_argfunction::Arguments_to_argfunction(
      const std::string& argfunction_type) {
  push_back("rwsh.arguments_for_argfunction");
  push_back(argfunction_type);}

Bad_argfunction_style::Bad_argfunction_style(
      const std::string& argfunction_style) {
  push_back("rwsh.bad_argfunction_style");
  push_back(argfunction_style);}

Bad_if_nest::Bad_if_nest() {push_back("rwsh.bad_if_nest");}

Divide_by_zero::Divide_by_zero(const std::string& value) {
  push_back("rwsh.divide_by_zero");
  push_back(value);}

Double_redirection::Double_redirection(const std::string& first, 
      const std::string& second) {
  push_back("rwsh.double_redirection");
  push_back(first);
  push_back(second);}

Else_without_if::Else_without_if() {push_back("rwsh.else_without_if");}

Excess_argfunction::Excess_argfunction() {
  push_back("rwsh.excess_argfunction");}

Failed_substitution::Failed_substitution(const std::string& function) {
  push_back("rwsh.failed_substitution");
  push_back(function);}

File_open_failure::File_open_failure(const std::string& file_name) {
  push_back("rwsh.file_open_failure");
  push_back(file_name);}

If_before_else::If_before_else() {push_back("rwsh.if_before_else");}

Invalid_word_selection::Invalid_word_selection(
      const std::string& selection) {
  push_back("rwsh.invalid_word_selection");
  push_back(selection);}

Input_range::Input_range(const std::string& value) {
  push_back("rwsh.input_range");
  push_back(value);}

Line_continuation::Line_continuation() {
  push_back("rwsh.line_continuation");}

Mismatched_brace::Mismatched_brace(const std::string& prefix) {
  push_back("rwsh.mismatched_brace");
  push_back(prefix);}

Mismatched_parenthesis::Mismatched_parenthesis(const std::string& prefix) {
  push_back("rwsh.mismatched_parenthesis");
  push_back(prefix);}

Multiple_argfunctions::Multiple_argfunctions() {
  push_back("rwsh.multiple_argfunctions");}

Not_soon_enough::Not_soon_enough(const std::string& argument) {
  push_back("rwsh.not_soon_enough");
  push_back(argument);}

Not_executable::Not_executable(const std::string& file_name) {
  push_back("rwsh.not_executable");
  push_back(file_name);}

Not_a_number::Not_a_number(const std::string& value) {
  push_back("rwsh.not_a_number");
  push_back(value);}

Result_range::Result_range(const std::string& lhs, const std::string& rhs) {
  push_back("rwsh.result_range");
  push_back(lhs);
  push_back(rhs);}

Unclosed_brace::Unclosed_brace(const std::string& prefix) {
  push_back("rwsh.subprompt");
  push_back(prefix);}

Unclosed_parenthesis::Unclosed_parenthesis(const std::string& prefix) {
  push_back("rwsh.subprompt");
  push_back(prefix);}

Undefined_variable::Undefined_variable(const std::string& variable) {
  push_back("rwsh.undefined_variable");
  push_back(variable);}
  
Old_argv::Old_argv(const Argv& src) : argc_v(src.size()) {
  focus = new char*[src.size()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv::~Old_argv(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}

