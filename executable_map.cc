// The definition of the Executable_map class, which defines the mapping
// between executable names and Executable objects. It takes an Argm as
// its key so that it can return argument functions which are part of the Argm
// object.
//
// Copyright (C) 2005-2017 Samuel Newbold

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
  else return nullptr;}

bool Executable_map::run_if_exists(const std::string& key, Argm& argm_i) {
  Argm temp_argm(key, argm_i.argv(), argm_i.argfunction(),
                 argm_i.parent_map(), argm_i.input,argm_i.output, argm_i.error);
  Base_executable* i = find(temp_argm);
  if (i) {
    Error_list exceptions;
    (*i)(temp_argm, exceptions);
    if (exceptions.size()) Base_executable::exception_handler(exceptions);
    return true;}
  else {
    return false;}}

int Executable_map::base_run(Argm& argm, Error_list& exceptions) {
  int ret = run(argm, exceptions);
  if (exceptions.size()) {
    Base_executable::exception_handler(exceptions);
    return -1;}
  else return ret;}

void Executable_map::unused_var_check_at_exit(void) {
  Error_list exceptions;
  Prototype shell_invocation(true);
  shell_invocation.unused_var_check(Variable_map::global_map, exceptions);
  if (exceptions.size()) Base_executable::exception_handler(exceptions);}

int Executable_map::run(Argm& argm, Error_list& exceptions) {
  try {
    Base_executable* i = find(argm);                    // first check for key
    if (i) return (*i)(argm, exceptions);
    else if (argm[0][0] == '/') {                       // insert a binary
      set(new Binary(argm[0]));
      return (*find(argm))(argm, exceptions);}
    if (is_function_name(argm[0])) {                    // try autofunction
      if (in_autofunction)                              // nested autofunction
        return not_found(argm, exceptions);
      in_autofunction = true;
      Argm auto_argm("rwsh.autofunction", argm.argv(),
                     argm.argfunction(), argm.parent_map(),
                     argm.input, argm.output, argm.error);
      run(auto_argm, exceptions);
      in_autofunction = false;
      i = find(argm);                                   // second check for key
      if (i) return (*i)(argm, exceptions);}
    return not_found(argm, exceptions);}
  catch (Exception error) {
    exceptions.add_error(error);
    return -1;}}

int Executable_map::not_found(Argm& argm_i, Error_list& exceptions) {
  Exception temp_argm(Argm::Function_not_found, argm_i[0]);
  Base_executable* i = find(temp_argm);
  if (!i) {                                      // reset executable_not_found
    std::string::size_type point = 0;
    set(new Function(Argm::exception_names[Argm::Function_not_found],
                     "{.echo $1 (: command not found) \\( $* \\) (\n)\n"
                     ".return -1}", point, 0, exceptions));}
  throw Exception(Argm::Function_not_found, argm_i);}

