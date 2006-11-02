// Functions to implement a variable map, and permit it to be exported as the
// environment for child processes.
//
// Copyright (C) 2006 Samuel Newbold

#include <assert.h>
#include <functional>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>

#include "argv.h"
#include "variable_map.h"

char** env;
Variable_map_t root_variable_map;
Variable_map_t* vars = &root_variable_map;
const std::string empty_str;
int Variable_map_t::dollar_question = -1;
int& dollar_question = Variable_map_t::dollar_question;
bool Variable_map_t::exit_requested = false;

const std::string& Variable_map_t::get(const std::string& key) {
  if (key == "?") {
    std::ostringstream tmp; 
    tmp <<dollar_question;
    (*this)["?"] = tmp.str();}
  std::map<std::string, std::string>::iterator i = find(key);
  if (i == end()) return empty_str;
  else return find(key)->second;}

void Variable_map_t::set(const std::string& key, const std::string& val) {
  if (val != empty_str) (*this)[key] = val;
  else {
    std::map<std::string, std::string>::iterator i = find(key);
    if (i != end()) erase(i);}
  if (key == "MAX_NESTING") {
    int temp = atoi((*this)[key].c_str());
    if (temp < 0) this->max_nesting = 0;
    else this->max_nesting = temp;}}

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
char** Variable_map_t::export_env(void) {
  delete env;
  env = new char*[vars->size() + 1];
  copy_to_char_star_star(this->begin(), this->end(), env);
  return env;}

