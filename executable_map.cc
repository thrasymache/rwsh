// The definition of the Executable_map_t class, which defines the mapping
// between executable names and Executable_t objects. It takes an Argv_t as its
// key so that it can return argument functions which are part of the Argv_t
// object.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <map>
#include <string>
#include <vector>

#include "argv.h"
#include "arg_script.h"
#include "builtin.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"

Executable_map_t::Executable_map_t(void) {
  set(new Built_in_t("%autofunction", autofunction_bi));
  set(new Built_in_t("%cd", cd_bi));
  set(new Built_in_t("%echo", echo_bi));
  set(new Built_in_t("%elif", elif_bi));
  set(new Built_in_t("%exit", exit_bi));
  set(new Built_in_t("%false", false_bi));
  set(new Built_in_t("%function", function_bi));
  set(new Built_in_t("%importenv", importenv_bi));
  set(new Built_in_t("%ls", ls_bi));
  set(new Built_in_t("%newline", newline_bi));
  set(new Built_in_t("%printenv", printenv_bi));
  set(new Built_in_t("%set", set_bi));
  set(new Built_in_t("%selection_set", selection_set_bi));
  set(new Built_in_t("%source", source_bi));
  set(new Built_in_t("%true", true_bi));
  set(new Built_in_t("%which", which_bi));
  set(new Built_in_t("%version", version_bi));}

// insert target into map, with key target->name(), replacing old value if key
// is already in the map
void Executable_map_t::set(Executable_t* target) {
  std::pair<iterator, bool> ret;
  ret = insert(std::make_pair(target->name(), target));
  if (!ret.second) { // replace if key already exists
    erase(ret.first);
    insert(std::make_pair(target->name(), target));}}

Executable_map_t::size_type Executable_map_t::erase (const Argv_t& key) {
  iterator pos = find(key);
  if (pos == end()) return 0;
  else {erase(pos); return 1;}}

void Executable_map_t::erase(iterator pos) {
  if (!pos->second->is_running()) delete pos->second;
  else pos->second->del_on_term = true;
  Base::erase(pos);}

Executable_map_t::iterator Executable_map_t::find(const Argv_t& key) {
  return Base::find(key[0]);}

// a normal map inserts a default value if a value is requested and it is not
// found. There is no useful purpose to inserting the fail function into the map
// it will waste memory, and slow the lookup of all subsequent requests. This
// implementation is in the spirit of a map because operator[] is returning
// a valid value regardless of the key requested. find() on the other hand
// returns a failure condition, an invalid iterator.
Executable_t& Executable_map_t::operator[] (Argv_t& key) {
  iterator i = this->find(key);                         // first check for key
  if (i != end()) return *i->second;
  else if (key[0][0] == '/') {                          // insert a binary
    set(new Binary_t(key[0]));
    return *this->find(key)->second;}
  else if (key[0] == "rwsh.mapped_argfunction") {       // handle argfunction
    if (key.argfunction()) return *key.argfunction();}
  key.push_front("rwsh.autofunction");                  // try autofunction
  i = this->find(key);
  if (i != end()) (*i->second)(key);
  key.pop_front();
  i = this->find(key);                                  // second check for key
  if (i != end()) return *i->second;
  key.push_front("rwsh.executable_not_found");          // executable_not_found
  i = this->find(key);
  if (i != end()) return *i->second;
  set(new Function_t("rwsh.executable_not_found", // reset executable_not_found
                     "%echo $1 : command not found ( $* ); %newline; %false"));
  return *(this->find(key)->second);}

