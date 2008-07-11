// The definition of the Executable_map_t class, which defines the mapping
// between executable names and Executable_t objects. It takes an Argv_t as its
// key so that it can return argument functions which are part of the Argv_t
// object.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

#include <map>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"

Executable_map_t::Executable_map_t(void) : in_autofunction(false) {}

// insert target into map, with key target->name(), replacing old value if key
// is already in the map
void Executable_map_t::set(Executable_t* target) {
  std::pair<iterator, bool> ret;
  ret = insert(std::make_pair(target->name(), target));
  if (!ret.second) { // replace if key already exists
    Base::erase(ret.first);
    insert(std::make_pair(target->name(), target));}}

Executable_map_t::size_type Executable_map_t::erase (const std::string& key) {
  iterator pos = Base::find(key);
  if (pos == end()) return 0;
  else {
    if (!pos->second->is_running()) delete pos->second;
    else pos->second->del_on_term = true;
    Base::erase(pos);
    return 1;}}

Executable_t* Executable_map_t::find(const Argv_t& key) {
  iterator i = Base::find(key[0]);
  if (i != end()) return i->second;
  else if (key[0] == "rwsh.mapped_argfunction") return key.argfunction(); 
  else return NULL;}

bool Executable_map_t::run_if_exists(const std::string& key, Argv_t& argv) {
  argv.push_front(key);
  Executable_t* i = find(argv);
  if (i) {
    (*i)(argv);
    if (Executable_t::unwind_stack()) Executable_t::signal_handler();
    argv.pop_front();
    return true;}
  else {
    argv.pop_front();
    return false;}}

int Executable_map_t::run(Argv_t& argv) {
  Executable_t* i = find(argv);                       // first check for key
  if (i) return (*i)(argv);
  else if (argv[0][0] == '/') {                       // insert a binary
    set(new Binary_t(argv[0]));
    return (*find(argv))(argv);}
  if (is_function_name(argv[0])) {                    // try autofunction
    if (in_autofunction) return not_found(argv);        // nested autofunction
    in_autofunction = true;
    run_if_exists("rwsh.autofunction", argv);
    in_autofunction = false;
    i = find(argv);                                   // second check for key
    if (i) return (*i)(argv);}
  return not_found(argv);}

int Executable_map_t::not_found(Argv_t& argv) {
  argv.push_front("rwsh.executable_not_found");         // executable_not_found
  Executable_t* i = find(argv);
  if (i) return (*i)(argv);
  set(new Function_t("rwsh.executable_not_found", // reset executable_not_found
                     "%echo $1 : command not found ( $* ); %newline; "
                     "%return -1", 0));
  return (*find(argv))(argv);}

