// The definition of the Argm class, which contains the arguments that may
// be passed to an executable.
//
// Copyright (C) 2005-2015 Samuel Newbold

#include <map>
#include <sstream>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argm.h"
#include "arg_script.h"
#include "executable.h"
#include "function.h"
#include "variable_map.h"

Argm::Argm(void) : argc_v(0), argfunction_v(0), 
  input(default_input), output(default_output), error(default_error),
  parent_map(Variable_map::global_map) {}

Argm::Argm(const Argm& src) : Base(src), argc_v(src.argc()),
  argfunction_v(src.argfunction()->copy_pointer()), 
  input(src.input), output(src.output), error(src.error),
  parent_map(Variable_map::global_map) {}

Argm& Argm::operator=(const Argm& src) {
  Base::clear(); 
  std::copy(src.begin(), src.end(), std::back_inserter(*this));
  argc_v = src.argc_v;
  delete argfunction_v;
  argfunction_v = src.argfunction()->copy_pointer();
  input = src.input;
  output = src.output;
  error = src.error;}

Argm::~Argm(void) {
  delete argfunction_v;}

// convert to a string. inverse of constructor.
std::string Argm::str(void) const {
  std::string result;
  for (const_iterator i=begin(); i != end()-1; ++i) result += *i + ' ';
  result += back();
  if (!input.is_default()) result += " " + input.str();
  if (!output.is_default()) result += " " + output.str();
  if (!error.is_default()) result += " " + error.str();
  if (argfunction()) result += " " + argfunction()->str();
  return result;}

void Argm::set_argfunction(Function* val) {argfunction_v = val;};

// returns variables that are defined in the current argument map other than $*
// (e.g. positional parameters and $#)
std::string Argm::get_var(const std::string& key) const {
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
        //throw Signal_argm(Argm::Undefined_variable, key);}
    default: return parent_map->get(key);}}

int Argm::set_var(const std::string& key, const std::string& value) const {
  switch (key[0]) {
    case '#': case '1': case '2': case '3': case '4': case '5': case '6': 
              case '7': case '8': case '9': case '0': return 2;
    default: return parent_map->set(key, value);}}

bool Argm::var_exists(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': return true; 
    case '1': case '2': case '3': case '4': case '5': case '6': 
              case '7': case '8': case '9': case '0': {
      int n = std::atoi(key.c_str());
      return size() > n;}
    default: return parent_map->exists(key);}}

int Argm::global_var(const std::string& key, 
                        const std::string& value) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5': 
              case '6': case '7': case '8': case '9': case '0': return 2;
    default: return parent_map->add(key, value);}}

int Argm::unset_var(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5': 
              case '6': case '7': case '8': case '9': case '0': return 3;
    default: return parent_map->unset(key);}}

unsigned Argm::max_nesting(void) const {return parent_map->max_nesting();}

char** Argm::export_env(void) const {return parent_map->export_env();}

// algorithm that is the meat of Old_argv constructor
template<class In>char** copy_to_cstr(In first, In last, char** res) {
  while (first != last) {
    *res = new char[(*first).length()+1];
    strcpy(*res++, (*first++).c_str());}
  *res = 0;
  return res;}

Signal_argm::Signal_argm(Sig_type signal_i) : signal(signal_i) {
  push_back(signal_names[signal]);}

Signal_argm::Signal_argm(Sig_type signal_i, const std::string& value) : 
    signal(signal_i) {
  push_back(signal_names[signal]);
  push_back(value);}

Signal_argm::Signal_argm(Sig_type signal_i, const std::string& x,
                         const std::string& y) : signal(signal_i) {
  push_back(signal_names[signal]);
  push_back(x);
  push_back(y);}

Signal_argm::Signal_argm(Sig_type signal_i, int x, int y) : signal(signal_i) {
  push_back(signal_names[signal]);
  std::ostringstream x_str, y_str;
  x_str <<x;
  push_back(x_str.str());
  y_str <<y;
  push_back(y_str.str());}

Signal_argm::Signal_argm(Sig_type signal_i, const Argm& src) : signal(signal_i){
  push_back(signal_names[signal]);
  std::copy(src.begin(), src.end(), std::back_inserter(*this));}

Old_argv::Old_argv(const Argm& src) : argc_v(src.argc()) {
  focus = new char*[src.argc()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv::~Old_argv(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}

