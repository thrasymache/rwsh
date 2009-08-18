// Functions to implement a variable map, and permit it to be exported as the
// environment for child processes.
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <map>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "executable.h"
#include "variable_map.h"

char** env;

Variable_map_t::Variable_map_t(bool root) : max_nesting_v(0) {
  if(root) {
    add("?", "");
    add("CWD", "");
    add("FIGNORE", "");
    add("MAX_NESTING", "0");}}

bool Variable_map_t::add(const std::string& key, const std::string& value) {
  std::pair<std::string, std::string> entry(key, value);
  std::pair<iterator, bool> ret = insert(entry);
  return !ret.second;}

bool Variable_map_t::exists(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator i = find(key);
  return i != end();}

const std::string& Variable_map_t::get(const std::string& key) {
  if (key == "?") {
    std::ostringstream tmp; 
    tmp <<dollar_question;
    (*this)["?"] = tmp.str();}
  std::map<std::string, std::string>::const_iterator i = find(key);
  if (i == end()) {
    Executable_t::caught_signal = Executable_t::SIGVAR;
    Executable_t::call_stack.push_back(key);
    throw Undefined_variable_t(key);}
  else return i->second;}

int Variable_map_t::set(const std::string& key, const std::string& value) {
  std::map<std::string, std::string>::iterator i = find(key);
  if (i == end()) {
    Executable_t::caught_signal = Executable_t::SIGVAR;
    Executable_t::call_stack.push_back(key);
    throw Undefined_variable_t(key);}
  else {
    i->second = value;
    if (key == "MAX_NESTING") {
      int temp = atoi(i->second.c_str());
      if (temp < 0) this->max_nesting_v = 0;
      else this->max_nesting_v = temp;}
    return 0;}}

int Variable_map_t::unset(const std::string& key) {
  if (key == "MAX_NESTING" || key == "FIGNORE" || key == "?" || 
      key == "CWD") return 2;
  std::map<std::string, std::string>::iterator i = find(key);
  if (i != end()) {erase(i); return 0;}
  else return 1;}

template <class In>
char** copy_to_char_star_star(In first, In last, char** res) {
  for (; first != last; ++first, ++res) {
    *res = new char[first->first.length() + first->second.length() + 2];
    strcpy(*res, first->first.c_str());
    (*res)[first->first.length()] = '=';
    strcpy(*res + first->first.length() + 1, first->second.c_str());}
  *res = 0;
  return res;}

extern Variable_map_t* vars;

// return the variable map in a way that can be passed to child processes
char** Variable_map_t::export_env(void) const {
  delete env;
  env = new char*[vars->size() + 1];
  copy_to_char_star_star(this->begin(), this->end(), env);
  return env;}

