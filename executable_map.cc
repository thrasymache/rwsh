// The definition of the Executable_map class, which defines the mapping
// between executable names and Executable objects. It takes an Argv as its
// key so that it can return argument functions which are part of the Argv
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

Executable_map::Executable_map(void) : in_autofunction(false) {}

// insert target into map, with key target->name(), replacing old value if key
// is already in the map
void Executable_map::set(Executable* target) {
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

Executable* Executable_map::find(const Argv& key) {
  iterator i = Base::find(key[0]);
  if (i != end()) return i->second;
  else if (key[0] == "rwsh.mapped_argfunction") return key.argfunction(); 
  else return NULL;}

bool Executable_map::run_if_exists(const std::string& key, Argv& argv) {
  argv.push_front(key);
  Executable* i = find(argv);
  if (i) {
    try {(*i)(argv);}
    catch (Signal_argv error) {
      std::copy(error.begin(), error.end(),
                std::back_inserter(Executable::call_stack));
      Executable::caught_signal = error.signal;
      Executable::signal_handler();
      return -1;}
    if (Executable::unwind_stack()) Executable::signal_handler();
    argv.pop_front();
    return true;}
  else {
    argv.pop_front();
    return false;}}

int Executable_map::run(Argv& argv) {
  try {
  Executable* i = find(argv);                       // first check for key
  if (i) return (*i)(argv);
  else if (argv[0][0] == '/') {                       // insert a binary
    set(new Binary(argv[0]));
    return (*find(argv))(argv);}
  if (is_function_name(argv[0])) {                    // try autofunction
    if (in_autofunction) return not_found(argv);        // nested autofunction
    in_autofunction = true;
    run_if_exists("rwsh.autofunction", argv);
    in_autofunction = false;
    i = find(argv);                                   // second check for key
    if (i) return (*i)(argv);}
  return not_found(argv);}
  catch (Signal_argv error) {
    std::copy(error.begin(), error.end(),
              std::back_inserter(Executable::call_stack));
    Executable::caught_signal = error.signal;
    if (!Executable::global_nesting && !Executable::in_signal_handler)
      Executable::signal_handler();
    return -1;}}

int Executable_map::not_found(Argv& argv) {
  argv.push_front(Argv::signal_names[Argv::Executable_not_found]);
  Executable* i = find(argv);
  if (!i) {
    std::string::size_type point = 0;
    set(new Function(Argv::signal_names[Argv::Executable_not_found],
                     "{.echo $1 (: command not found) \\( $* \\) (\n)\n"
                     ".return -1}", point, 0));}   // reset executable_not_found
  argv.pop_front();
  throw Signal_argv(Argv::Executable_not_found, argv);}

