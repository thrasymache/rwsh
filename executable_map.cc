// The definition of the Executable_map class, which defines the mapping
// between executable names and Executable objects. It takes an Argm as
// its key so that it can return argument functions which are part of the Argm
// object.
//
// Copyright (C) 2005-2019 Samuel Newbold

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
#include "call_stack.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"
#include "tokenize.cc"

#include "function.h"

Executable_map::Executable_map(void) : in_autofunction(false) {}

// insert target into map, with key target->name(), replacing old value if key
// is already in the map
void Executable_map::set(Named_executable* target) {
  std::pair<iterator, bool> ret;
  ret = insert(std::make_pair(target->name(), target));
  if (!ret.second) { // replace if key already exists
    if (!ret.first->second->is_running()) delete ret.first->second;
    else ret.first->second->del_on_term = true;
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

Base_executable* Executable_map::find_second(const Argm& key) {
  iterator i = Base::find(key[0]);
  if (i != end()) return i->second;
  else if (key[0] == ".mapped_argfunction") return key.argfunction();
  else return nullptr;}

bool Executable_map::run_if_exists(const std::string& key, Argm& argm_i) {
  Argm temp_argm(key, argm_i.argv(), argm_i.argfunction(),
                 argm_i.parent_map(), argm_i.input,argm_i.output, argm_i.error);
  Base_executable* i = find_second(temp_argm);
  if (i) {
    Error_list exceptions;
    (*i)(temp_argm, exceptions);
    if (global_stack.unwind_stack())
      global_stack.exception_handler(exceptions);
    return true;}
  else {
    return false;}}

void Executable_map::base_run(Argm& argm, Error_list& exceptions) {
  run(argm, exceptions);
  if (gc_state.in_if_block && !gc_state.exception_thrown) {
    gc_state.in_if_block = false;
    exceptions.add_error(Exception(Argm::Unfinished_if_block));}
  if (global_stack.unwind_stack())
    global_stack.exception_handler(exceptions);}

void Executable_map::run_handling_exceptions(Argm& argm,
                                             Error_list& exceptions) {
  run(argm, exceptions);
  if (global_stack.unwind_stack())
    global_stack.exception_handler(exceptions);}

void Executable_map::unused_var_check_at_exit(void) {
  Error_list exceptions;
  Prototype shell_invocation(true);
  shell_invocation.unused_var_check(Variable_map::global_map, exceptions);
  if (exceptions.size()) global_stack.exception_handler(exceptions);}

void Executable_map::run(Argm& argm, Error_list& exceptions) {
  try {
    Base_executable* i = find_second(argm);             // first check for key
    if (i) (*i)(argm, exceptions);
    else if (in_autofunction)                           // nested autofunction
      not_found(argm, exceptions);
    else {
      in_autofunction = true;
      Argm auto_argm(Argm::exception_names[Argm::Autofunction], argm.argv(),
                     argm.argfunction(), argm.parent_map(),
                     argm.input, argm.output, argm.error);
      run(auto_argm, exceptions);
      in_autofunction = false;
      i = find_second(argm);                            // second check for key
      if (i) (*i)(argm, exceptions);
      else not_found(argm, exceptions);}}
  catch (Exception error) {
    exceptions.add_error(error);}}

bool Executable_map::run_condition(Argm& argm, Error_list& exceptions) {
  run(argm, exceptions);
  return !global_stack.remove_exceptions(".false", exceptions) &&
         !exceptions.size();}  // optional

void Executable_map::not_found(Argm& argm_i, Error_list& exceptions) {
  if (Base::find(Argm::exception_names[Argm::Function_not_found]) == end()) {
    Argm prototype_argm(argm_i.parent_map(), argm_i.input, argm_i.output,
                        argm_i.error);
    tokenize_words("cmd [args ...]", std::back_inserter(prototype_argm));
    std::string::size_type point = 0;
    Command_block body("{.echo $cmd (: command not found) "
                       "\\( $cmd $args$ \\); .echo (\n)}",
                       point, 0, exceptions);
    set(new Function(Argm::exception_names[Argm::Function_not_found],
                     prototype_argm.argv(), body));}
  throw Exception(Argm::Function_not_found, argm_i);}

