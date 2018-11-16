// Functions to implement a variable map, and permit it to be exported as the
// environment for child processes.
//
// Copyright (C) 2006-2018 Samuel Newbold

#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <set>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "executable.h"

std::string escape(const std::string& src) {
  if (!src.length()) return "()";
  else return src;}

std::string word_from_value(const std::string& value) {
  if (value == "") return std::string("()");
  else if (value.find_first_of(" \t\n") != std::string::npos)
    return "(" + escape(value) + ")";
  else return escape(value);}

Variable_map::Variable_map(Variable_map* parent_i) :
    parent(parent_i), locals_listed(false), usage_checked(false) {
  if (parent_i == nullptr) {
    param("?", "");               // ? does not need to be used (in subshells)
    local("FIGNORE", "");}}

void Variable_map::bless_unused_vars() {
  if (!usage_checked) usage_checked = true;
  else throw Exception(Argm::Internal_error,
                       "variable map usage checked multiple times");
  for (auto i: *this) if (used_vars.find(i.first) == used_vars.end())
    used_vars.insert(i.first);}

Variable_map::~Variable_map() {
  if (!usage_checked)  // variable map usage not checked, this has found a bug
    std::abort();}

void Variable_map::append_word_locally(const std::string& key,
                                       const std::string& value) {
  auto i = find(key);
  if (i == end()) std::abort();  //prototype should only append to what exists
  else i->second += " " + word_from_value(value);}

void Variable_map::append_word_if_exists(const std::string& key,
                                         const std::string& value) {
  auto i = find(key);
  if (i == end()) return;
  else if (i->second == "") i->second = word_from_value(value);
  else i->second += " " + word_from_value(value);}

void Variable_map::param_or_append_word(const std::string& key,
                                        const std::string& value) {
  auto i = find(key);
  if (i == end()) param(key, word_from_value(value));
  else i->second += " " + word_from_value(value);}

bool Variable_map::exists(const std::string& key, bool check) {
  if (check) checked_vars.insert(key);
  if (simple_exists(key)) return true;
  else if (undefined_vars.find(key) != undefined_vars.end()) return false;
  else if (parent) return parent->exists(key, check);
  else return false;}

const std::string& Variable_map::get(const std::string& key) {
  if (key == "?") {
    std::ostringstream tmp;
    tmp <<dollar_question;
    (*this)["?"] = tmp.str();}
  auto i = find(key);
  if (i != end()) {
    used_vars.insert(key);
    return i->second;}
  else if (undefined_vars.find(key) != undefined_vars.end())
    throw Undefined_variable(key);
  else if (parent) return parent->get(key);
  else throw Exception(Argm::Undeclared_variable, key);}

void Variable_map::global(const std::string& key, const std::string& value) {
  if (parent)
    if (find(key) != end()) throw Exception(Argm::Global_would_be_masked, key);
    else if (usage_checked)
      throw Exception(Argm::Internal_error,
                      "variable map added to after usage checked");
    else parent->global(key, value);
  else local(key, value);}

// params have their usage checked by the prototype (to properly handle -* etc)
void Variable_map::param(const std::string& key, const std::string& value) {
  std::pair<std::string, std::string> entry(key, value);
  std::pair<iterator, bool> ret = insert(entry);
  if (!ret.second && value != ret.first->second)
    throw Exception(Argm::Variable_already_exists, key);}

// locals have their usage checked directly
void Variable_map::local(const std::string& key, const std::string& value) {
  if (usage_checked)
    throw Exception(Argm::Internal_error,
                    "variable map added to after usage checked");
  local_vars.insert(key);
  param(key, value);}

void Variable_map::set(const std::string& key, const std::string& value) {
  auto i = find(key);
  if (i != end())
    if (used_vars.find(key) == used_vars.end() && i->second != value) {
      used_vars.insert(key);  // we're about to throw a more specific error
      throw Exception(Argm::Unused_before_set, key);}
    else i->second = value;
  else if (undefined_vars.find(key) != undefined_vars.end()) param(key, value);
  else if (parent) parent->set(key, value);
  else throw Exception(Argm::Undeclared_variable, key);}

void Variable_map::unset(const std::string& key) {
  if (key == "FIGNORE" || key == "?")
    throw Exception(Argm::Illegal_variable_name, key);
  auto i = find(key);
  if (i != end()) erase(i);
  else if (parent) parent->unset(key);
  else throw Exception(Argm::Undeclared_variable, key);}

const Variable_map* Variable_map::parent_with(const std::string& key) const {
  auto i = find(key);
  if (i != end()) return this;
  else if(parent) return parent->parent_with(key);
  else return nullptr;}

template <class In, class Out>
Out Variable_map::copy_to_char_star_star(
        In first, In last, Out res, const Variable_map* descendant) {
  for (; first != last; ++first) {
    std::string key = first->first;
    if (descendant->parent_with(key) != this) continue;
    used_vars.insert(key);
    std::string value = get(key);
    auto focus = new char[key.length() + value.length() + 2];
    std::strcpy(focus, key.c_str());
    focus[key.length()] = '=';
    std::strcpy(focus + key.length() + 1, value.c_str());
    *res++ = focus;}
  return res;}

// return the variable map in a way that can be passed to child processes
void Variable_map::export_env(std::vector<char*>& env) {
  export_env(env, this);
  env.push_back(nullptr);}

void Variable_map::export_env(
        std::vector<char*>& env, const Variable_map* descendant) {
  if (parent) parent->export_env(env, descendant);
  copy_to_char_star_star(begin(), end(), std::back_inserter(env), descendant);}
