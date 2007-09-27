// The definition of the Argv_t class, which contains the arguments that may
// be passed to an executable.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <map>
#include <sstream>
#include <vector>

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "function.h"
#include "rwsh_stream.h"
#include "variable_map.h"

extern Variable_map_t* vars;
Variable_map_t* Argv_t::var_map = vars;

Argv_t::Argv_t(void) : argfunction_v(0), myout_v(default_stream_p) {};

Argv_t::Argv_t(const Argv_t& src) : Base(src), argfunction_v(0), 
      myout_v(src.myout_v) {
  argfunction_v = src.argfunction()->copy_pointer();}

Argv_t& Argv_t::operator=(const Argv_t& src) {
  clear();
  std::copy(src.begin(), src.end(), std::back_inserter(*this));
  argfunction_v = src.argfunction()->copy_pointer();
  myout_v = src.myout_v;}

Argv_t::~Argv_t(void) {delete argfunction_v;}

// convert to a string. inverse of constructor.
std::string Argv_t::str(void) const {
  std::string result;
  for (const_iterator i=begin(); i != end()-1; ++i) result += *i + ' ';
  result += back();
  if (argfunction()) result += " " + argfunction()->str();
  return result;}

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

void Argv_t::clear(void) {
  Base::clear(); delete argfunction_v; argfunction_v=0;}

// algorithm that is the meat of Old_argv constructor
template<class In>char** copy_to_cstr(In first, In last, char** res) {
  while (first != last) {
    *res = new char[(*first).length()+1];
    strcpy(*res++, (*first++).c_str());}
  *res = 0;
  return res;}

Old_argv_t::Old_argv_t(const Argv_t& src) : argc_v(src.size()) {
  focus = new char*[src.size()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv_t::~Old_argv_t(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}

