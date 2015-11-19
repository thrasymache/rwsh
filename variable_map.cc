// Functions to implement a variable map, and permit it to be exported as the
// environment for child processes.
//
// Copyright (C) 2006-2015 Samuel Newbold

#include <cstring>
#include <map>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "executable.h"

char** env;

Variable_map::Variable_map(Variable_map* parent_i) : parent(parent_i) {
  if (parent_i == NULL) {
    local("?", "");
    local("FIGNORE", "");
    local("MAX_NESTING", "0");}}

void Variable_map::append_word_locally(const std::string& key,
                                       const std::string& value) {
  std::string word(value.find_first_of(" \t\n") == std::string::npos?
                                                     value: "(" + value + ")");
  std::map<std::string, std::string>::iterator i = find(key);
  if (i == end()) throw Signal_argm(Argm::Undefined_variable, key);
  else i->second += " " + word;}

void Variable_map::append_word_if_exists(const std::string& key,
                                         const std::string& value) {
  std::string word(value.find_first_of(" \t\n") == std::string::npos?
                                                     value: "(" + value + ")");
  std::map<std::string, std::string>::iterator i = find(key);
  if (i == end()) return;
  else if (i->second == "") i->second = word;
  else i->second += " " + word;}

void Variable_map::local_or_append_word(const std::string& key,
                                        const std::string& value) {
  std::string word(value.find_first_of(" \t\n") == std::string::npos?
                                                     value: "(" + value + ")");
  std::map<std::string, std::string>::iterator i = find(key);
  if (i == end()) local(key, word);
  else i->second += " " + word;}

bool Variable_map::exists(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator i = find(key);
  if (i != end()) return true;
  else if(parent) return parent->exists(key);
  else return false;}

const std::string& Variable_map::get(const std::string& key) {
  if (key == "?") {
    std::ostringstream tmp; 
    tmp <<dollar_question;
    (*this)["?"] = tmp.str();}
  std::map<std::string, std::string>::const_iterator i = find(key);
  if (i != end()) return i->second;
  else if (parent) return parent->get(key);
  else throw Signal_argm(Argm::Undefined_variable, key);}

int Variable_map::global(const std::string& key, const std::string& value) {
  if (parent)
    if (find(key) != end()) return 3;
    else return parent->global(key, value);
  else return local(key, value);}

int Variable_map::local(const std::string& key, const std::string& value) {
  std::pair<std::string, std::string> entry(key, value);
  std::pair<iterator, bool> ret = insert(entry);
  return !ret.second;}

void Variable_map::set(const std::string& key, const std::string& value) {
  std::map<std::string, std::string>::iterator i = find(key);
  if (i != end()) {
    i->second = value;
    if (key == "MAX_NESTING") {
      int temp = atoi(i->second.c_str());
      if (temp < 0) this->max_nesting_v = 0;
      else this->max_nesting_v = temp;}}
  else if (parent) parent->set(key, value);
  else throw Signal_argm(Argm::Undefined_variable, key);}

int Variable_map::unset(const std::string& key) {
  if (key == "MAX_NESTING" || key == "FIGNORE" || key == "?") return 2;
  std::map<std::string, std::string>::iterator i = find(key);
  if (i != end()) {erase(i); return 0;}
  else if (parent) return parent->unset(key);
  else return 1;}

template <class In>
char** Variable_map::copy_to_char_star_star(In first, In last, char** res) {
  for (; first != last; ++first, ++res) {
    std::string key = first->first;
    std::string value = get(key);
    *res = new char[key.length() + value.length() + 2];
    std::strcpy(*res, key.c_str());
    (*res)[key.length()] = '=';
    std::strcpy(*res + key.length() + 1, value.c_str());}
  *res = 0;
  return res;}

// return the variable map in a way that can be passed to child processes
char** Variable_map::export_env(void) {
  if (parent) return parent->export_env();
  delete env;
  env = new char*[global_map->size() + 1];
  copy_to_char_star_star(this->begin(), this->end(), env);
  return env;}

