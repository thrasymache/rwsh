// The definition of the Argv_t class, which contains the arguments that may
// be passed to an executable.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "function.h"
#include "tokenize.cc"
#include "util.h"

// algorithm used twice by string constructor
void Argv_t::add_tokens(const std::string& src) {
  std::string::size_type skipspace = src.find_first_not_of(" "); 
  if (skipspace != std::string::npos) {
    tokenize(src.substr(skipspace), std::back_inserter(*this), 
             std::bind2nd(std::equal_to<char>(), ' '));}}

Argv_t::Argv_t(const std::string& src) : argfunction_v(0) {
  std::string::size_type o_brace, c_brace;
  o_brace = src.find_first_of("{}"); 
  add_tokens(src.substr(0, o_brace));
  while (o_brace != std::string::npos) {
    c_brace = find_close_brace(src, o_brace);
    if (src[o_brace] == '}' || c_brace == std::string::npos) {
      clear();
      push_back("rwsh.mismatched_brace");
      push_back(src.substr(0, o_brace+1));
      return;}
    if (argfunction_v) {
      clear();
      push_back("rwsh.multiple_argfunctions");
      return;}
    argfunction_v = new Function_t("rwsh.argfunction", 
                                   src.substr(o_brace+1, c_brace-o_brace-1));
    o_brace = src.find_first_of("{}", c_brace+1);
    add_tokens(src.substr(c_brace+1, o_brace-c_brace-1));};}

Argv_t::Argv_t(const Argv_t& src) : Base(src), argfunction_v(0) {
  argfunction_v = src.argfunction()->copy_pointer();}

Argv_t& Argv_t::operator=(const Argv_t& src) {
  clear();
  copy(src.begin(), src.end(), std::back_inserter(*this));
  argfunction_v = src.argfunction()->copy_pointer();}

Argv_t::~Argv_t(void) {delete argfunction_v;}

// convert to a string. inverse of constructor.
std::string Argv_t::str(void) const {
  std::string result;
  for (const_iterator i=begin(); i != end()-1; ++i) result += *i + ' ';
  result += back();
  if (argfunction()) result += " " + argfunction()->str();
  return result;}

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
      else return "";}}}

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

