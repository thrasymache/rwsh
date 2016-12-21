// The definition of the Executable_map class, which defines the mapping
// between executable names and Executable objects. It takes an Argm as
// its key so that it can return argument functions which are part of the Argm
// object.
//
// Copyright (C) 2005-2016 Samuel Newbold

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

Executable_map::Executable_map(void) : in_autofunction(false) {}

// insert target into map, with key target->name(), replacing old value if key
// is already in the map
void Executable_map::set(Named_executable* target) {
  std::pair<iterator, bool> ret;
  ret = insert(std::make_pair(target->name(), target));
  if (!ret.second) { // replace if key already exists
    Base::erase(ret.first);
    insert(std::make_pair(target->name(), target));}}

Executable_map::size_type Executable_map::erase (const std::string& key) {
  iterator pos = Base::find(key);
  if (pos == end()) return 0;
  else {
    if (!pos->second->is_running()) delete pos->second;
    else pos->second->del_on_term = true;
    Base::erase(pos);
    return 1;}}

Base_executable* Executable_map::find(const Argm& key) {
  iterator i = Base::find(key[0]);
  if (i != end()) return i->second;
  else if (key[0] == "rwsh.mapped_argfunction") return key.argfunction();
  else return NULL;}

bool Executable_map::run_if_exists(const std::string& key, Argm& argm_i) {
  Argm temp_argm(key, argm_i.begin(), argm_i.end(), argm_i.argfunction(),
                 argm_i.parent_map(), argm_i.input,argm_i.output, argm_i.error);
  Base_executable* i = find(temp_argm);
  if (i) {
    std::list<Argm> exceptions;
    (*i)(temp_argm, exceptions);
    if (exceptions.size()) Base_executable::exception_handler(exceptions);
    return true;}
  else {
    return false;}}

int Executable_map::base_run(Argm& argm) {
  std::list<Argm> exceptions;
  int ret = run(argm, exceptions);
  if (exceptions.size()) {
    Base_executable::exception_handler(exceptions);
    return -1;}
  else return ret;}

int Executable_map::run(Argm& argm, std::list<Argm>& exceptions) {
  try {
    Base_executable* i = find(argm);                    // first check for key
    if (i) return (*i)(argm, exceptions);
    else if (argm[0][0] == '/') {                       // insert a binary
      set(new Binary(argm[0]));
      return (*find(argm))(argm, exceptions);}
    if (is_function_name(argm[0])) {                    // try autofunction
      if (in_autofunction) return not_found(argm);        // nested autofunction
      in_autofunction = true;
      Argm auto_argm("rwsh.autofunction", argm.begin(), argm.end(),
                     argm.argfunction(), argm.parent_map(),
                     argm.input, argm.output, argm.error);
      run(auto_argm, exceptions);
      in_autofunction = false;
      i = find(argm);                                   // second check for key
      if (i) return (*i)(argm, exceptions);}
    return not_found(argm);}
  catch (Exception error) {
    Base_executable::add_error(exceptions, error);
    return -1;}}

int Executable_map::not_found(Argm& argm_i) {
  Exception temp_argm(Argm::Executable_not_found, argm_i[0]);
  Base_executable* i = find(temp_argm);
  if (!i) {
    std::string::size_type point = 0;
    set(new Function(Argm::exception_names[Argm::Executable_not_found],
                     "{.echo $1 (: command not found) \\( $* \\) (\n)\n"
                     ".return -1}", point, 0));}   // reset executable_not_found
  throw Exception(Argm::Executable_not_found, argm_i);}

