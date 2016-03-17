// The definition of the Executable_map class, which defines the mapping
// between executable names and Named_executable objects. It takes an Argm as
// its key so that it can return argument functions which are part of the Argm
// object.
//
// Copyright (C) 2005-2016 Samuel Newbold

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

Named_executable* Executable_map::find(const Argm& key) {
  iterator i = Base::find(key[0]);
  if (i != end()) return i->second;
  else if (key[0] == "rwsh.mapped_argfunction") return key.argfunction(); 
  else return NULL;}

bool Executable_map::run_if_exists(const std::string& key, Argm& argm_i) {
  Argm temp_argm(key, argm_i.begin(), argm_i.end(), argm_i.argfunction(),
                 argm_i.parent_map(), argm_i.input,argm_i.output, argm_i.error);
  Named_executable* i = find(temp_argm);
  if (i) {
    (*i)(temp_argm);
    if (Named_executable::unwind_stack()) Base_executable::signal_handler();
    return true;}
  else {
    return false;}}

int Executable_map::run(Argm& argm) {
  try {
    Named_executable* i = find(argm);                   // first check for key
    if (i) return (*i)(argm);
    else if (argm[0][0] == '/') {                       // insert a binary
      set(new Binary(argm[0]));
      return (*find(argm))(argm);}
    if (is_function_name(argm[0])) {                    // try autofunction
      if (in_autofunction) return not_found(argm);        // nested autofunction
      in_autofunction = true;
      run_if_exists("rwsh.autofunction", argm);
      in_autofunction = false;
      i = find(argm);                                   // second check for key
      if (i) return (*i)(argm);}
    return not_found(argm);}
  catch (Signal_argm error) {
    Base_executable::caught_signal = error.signal;
    std::copy(error.begin(), error.end(),
              std::back_inserter(Base_executable::call_stack));
    if (!Base_executable::global_nesting && !Base_executable::in_signal_handler)
      Base_executable::signal_handler();
    return -1;}}

int Executable_map::not_found(Argm& argm_i) {
  Signal_argm temp_argm(Argm::Executable_not_found, argm_i[0]);
  Named_executable* i = find(temp_argm);
  if (!i) {
    std::string::size_type point = 0;
    set(new Function(Argm::signal_names[Argm::Executable_not_found],
                     "{.echo $1 (: command not found) \\( $* \\) (\n)\n"
                     ".return -1}", point, 0));}   // reset executable_not_found
  throw Signal_argm(Argm::Executable_not_found, argm_i);}

