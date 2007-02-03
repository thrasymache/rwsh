// Functions to implement a variable map, and permit it to be exported as the
// environment for child processes.
//
// Copyright (C) 2006 Samuel Newbold

#include <map>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>

#include "argv.h"
#include "variable_map.h"

char** env;
Variable_map_t root_variable_map(true);
Variable_map_t* vars = &root_variable_map;
const std::string empty_str;
int Variable_map_t::dollar_question = -1;
int& dollar_question = Variable_map_t::dollar_question;
bool Variable_map_t::exit_requested = false;

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

void Variable_map_t::append_to_errno(const std::string& value) {
  if (exists("ERRNO")) set("ERRNO", get("ERRNO") + " " + value);
  else add("ERRNO", value);}

bool Variable_map_t::exists(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator i = find(key);
  return i != end();}

const std::string& Variable_map_t::get(const std::string& key) {
  if (key == "?") {
    std::ostringstream tmp; 
    tmp <<dollar_question;
    (*this)["?"] = tmp.str();}
  std::map<std::string, std::string>::const_iterator i = find(key);
  if (i == end()) return empty_str;
  else return find(key)->second;}

void Variable_map_t::set(const std::string& key, const std::string& val) {
  (*this)[key] = val;
  if (key == "MAX_NESTING") {
    int temp = atoi((*this)[key].c_str());
    if (temp < 0) this->max_nesting_v = 0;
    else this->max_nesting_v = temp;}}

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

// return the variable map in a way that can be passed to child processes
char** Variable_map_t::export_env(void) const {
  delete env;
  env = new char*[vars->size() + 1];
  copy_to_char_star_star(this->begin(), this->end(), env);
  return env;}

