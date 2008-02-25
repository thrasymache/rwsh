// The definition of the Argv_t class, which contains the arguments that may
// be passed to an executable.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <map>
#include <sstream>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "function.h"
#include "variable_map.h"

extern Variable_map_t* vars;
Variable_map_t* Argv_t::var_map = vars;

Argv_t::Argv_t(void) : argfunction_v(0), streams(3) {}

Argv_t::Argv_t(const Argv_t& src) : Base(src), 
  argfunction_v(src.argfunction()->copy_pointer()), 
  streams(src.streams.begin(), src.streams.end()) {;}

Argv_t& Argv_t::operator=(const Argv_t& src) {
  clear();
  std::copy(src.begin(), src.end(), std::back_inserter(*this));
  std::copy(src.streams.begin(), src.streams.end(), 
            streams.begin());
  argfunction_v = src.argfunction()->copy_pointer();}

Argv_t::~Argv_t(void) {
  delete argfunction_v;}

// convert to a string. inverse of constructor.
std::string Argv_t::str(void) const {
  std::string result;
  for (const_iterator i=begin(); i != end()-1; ++i) result += *i + ' ';
  result += back();
  for (std::vector<Rwsh_stream_p>::const_iterator i = streams.begin();
       i != streams.end(); ++i) if (!i->is_default())
    result += " " + i->str();
  if (argfunction()) result += " " + argfunction()->str();
  return result;}

void Argv_t::set_stream(int fileno, const Rwsh_stream_p& val) {
  assert(fileno >= 0 && fileno < streams.size());
  streams[fileno] = val;};

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

void Argv_t::clear(void) {
  Base::clear(); 
  delete argfunction_v; argfunction_v=0; 
  streams[0] = Rwsh_stream_p();
  streams[1] = Rwsh_stream_p();
  streams[2] = Rwsh_stream_p();}

// algorithm that is the meat of Old_argv constructor
template<class In>char** copy_to_cstr(In first, In last, char** res) {
  while (first != last) {
    *res = new char[(*first).length()+1];
    strcpy(*res++, (*first++).c_str());}
  *res = 0;
  return res;}

Arguments_to_argfunction_t::Arguments_to_argfunction_t(
      const std::string& argfunction_type) : Argv_t() {
  push_back("rwsh.arguments_for_argfunction");
  push_back(argfunction_type);}

Bad_argfunction_style_t::Bad_argfunction_style_t(
      const std::string& argfunction_style) : Argv_t() {
  push_back("rwsh.bad_argfunction_style");
  push_back(argfunction_style);}

Double_redirection_t::Double_redirection_t(const std::string& first, 
      const std::string& second) : Argv_t() {
  push_back("rwsh.double_redirection");
  push_back(first);
  push_back(second);}

Failed_substitution_t::Failed_substitution_t(const std::string& function) :
      Argv_t() {
  push_back("rwsh.failed_substitution");
  push_back(function);}

File_open_failure_t::File_open_failure_t(const std::string& file_name) {
  push_back("rwsh.file_open_failure");
  push_back(file_name);}

Mismatched_brace_t::Mismatched_brace_t(const std::string& prefix) : 
      Argv_t() {
  push_back("rwsh.mismatched_brace");
  push_back(prefix);}

Multiple_argfunctions_t::Multiple_argfunctions_t() : Argv_t() {
  push_back("rwsh.multiple_argfunctions");}

Not_soon_enough_t::Not_soon_enough_t(const std::string& argument) : 
      Argv_t() {
  push_back("rwsh.not_soon_enough");
  push_back(argument);}

Undefined_variable_t::Undefined_variable_t(const std::string& variable) :
      Argv_t() {
  push_back("rwsh.undefined_variable");
  push_back(variable);}
  
Old_argv_t::Old_argv_t(const Argv_t& src) : argc_v(src.size()) {
  focus = new char*[src.size()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv_t::~Old_argv_t(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}

