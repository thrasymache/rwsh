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
std::map<std::string, std::string> variable_map;
int dollar_question;
unsigned max_nesting = 0;
bool exit_requested = false;

const std::string& get_var(const std::string& key) {
  assert(!isargvar(key));
  if (key == "?") {
    std::ostringstream tmp; 
    tmp <<dollar_question;
    variable_map["?"] = tmp.str();}
  return variable_map[key];}

void set_var(const std::string& key, const std::string& val) {
  assert(!isargvar(key));
  variable_map[key] = val;
  if (key == "MAX_NESTING") {
    int temp = atoi(variable_map[key].c_str());
    if (temp < 0) max_nesting = 0;
    else max_nesting = temp;}}

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
char** export_env(void) {
  delete env;
  env = new char*[variable_map.size() + 1];
  copy_to_char_star_star(variable_map.begin(), variable_map.end(), env);
  return env;}

void print_var(std::pair<std::string, std::string> src) {
  std::cout <<src.first <<'=' <<src.second <<std::endl;}

// builtin function for printing the variable map
int printenv_bi(const Argv_t& argv) {
  if (argv.size() < 2) {
    for_each(variable_map.begin(), variable_map.end(), print_var);
    return 0;}
  else {
    std::cout <<variable_map[argv[1]] <<std::endl;
    return 0;}}

