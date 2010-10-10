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
      else return std::string();}
        //throw Signal_argv(Argv::Undefined_variable, key);}
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

Signal_argv::Signal_argv(Sig_type signal_i) : signal(signal_i) {
  push_back(signal_names[signal]);}

Signal_argv::Signal_argv(Sig_type signal_i, const std::string& value) : 
    signal(signal_i) {
  push_back(signal_names[signal]);
  push_back(value);}

Signal_argv::Signal_argv(Sig_type signal_i, const std::string& x,
                         const std::string& y) :
    signal(signal_i) {
  push_back(signal_names[signal]);
  push_back(x);
  push_back(y);}

Signal_argv::Signal_argv(Sig_type signal_i, int x, int y) :
    signal(signal_i) {
  push_back(signal_names[signal]);
  std::ostringstream x_str, y_str;
  x_str <<x;
  push_back(x_str.str());
  y_str <<y;
  push_back(y_str.str());}

Signal_argv::Signal_argv(Sig_type signal_i, const Argv& src) {
  signal = signal_i;
  push_back(signal_names[signal]);
  //Executable::caught_signal = signal;
  std::copy(src.begin(), src.end(), std::back_inserter(*this));}

Old_argv::Old_argv(const Argv& src) : argc_v(src.size()) {
  focus = new char*[src.size()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv::~Old_argv(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}

