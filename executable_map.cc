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
  set(new Builtin_t("%autofunction", autofunction_bi));
  set(new Builtin_t("%cd", cd_bi));
  set(new Builtin_t("%echo", echo_bi));
  set(new Builtin_t("%else", else_bi));
  set(new Builtin_t("%else_if", else_if_bi));
  set(new Builtin_t("%else_if_not", else_if_not_bi));
  set(new Builtin_t("%exit", exit_bi));
  set(new Builtin_t("%for", for_bi));
  set(new Builtin_t("%function", function_bi));
  set(new Builtin_t("%global", global_bi));
  set(new Builtin_t("%if", if_bi));
  set(new Builtin_t("%if_errno", if_errno_bi));
  set(new Builtin_t("%importenv_overwrite", importenv_overwrite_bi));
  set(new Builtin_t("%importenv_preserve", importenv_preserve_bi));
  set(new Function_t("%internal_errors", 
                     "%if %test_equal $# 1 {"
                         "%echo rwsh.arguments_for_argfunction "
                         "rwsh.binary_not_found rwsh.executable_not_found "
                         "rwsh.excessive_nesting rwsh.mismatched_brace "
                         "rwsh.multiple_argfunctions rwsh.init "
                         "rwsh.selection_not_found rwsh.sighup rwsh.sigint "
                         "rwsh.sigquit rwsh.sigpipe rwsh.sigterm rwsh.sigstp "
                         "rwsh.sigcont rwsh.siginfo rwsh.sigusr1 rwsh.sigusr2 "
                         "rwsh.unreadable_dir}; "
                     "%else {%set ERRNO ARGS; %return -1}"));
  set(new Function_t("%internal_features", 
                     "%if %test_equal $# 1 {"
                         "%echo rwsh.after_command rwsh.before_command "
                         "rwsh.prompt rwsh.raw_command rwsh.run_logic "
                         "rwsh.shutdown rwsh.vars}; "
                     "%else {%set ERRNO ARGS; %return -1}"));
  set(new Function_t("%internal_vars", 
                     "%if %test_equal $# 1 {%echo CWD ERRNO FIGNORE IF_TEST}; "
                     "%else {%set ERRNO ARGS; %return -1}"));
  set(new Builtin_t("%ls", ls_bi));
  set(new Builtin_t("%newline", newline_bi));
  set(new Builtin_t("%nop", nop_bi));
  set(new Builtin_t("%printenv", printenv_bi));
  set(new Builtin_t("%return", return_bi));
  set(new Builtin_t("%set", set_bi));
  set(new Builtin_t("%selection_set", selection_set_bi));
  set(new Builtin_t("%stepwise", stepwise_bi));
  set(new Builtin_t("%source", source_bi));
  set(new Builtin_t("%test_equal", test_equal_bi));
  set(new Builtin_t("%test_not_empty", test_not_empty_bi));
  set(new Builtin_t("%test_not_equal", test_not_equal_bi));
  set(new Builtin_t("%unset", unset_bi));
  set(new Builtin_t("%which_executable", which_executable_bi));
  set(new Builtin_t("%which_path", which_path_bi));
  set(new Builtin_t("%which_return", which_return_bi));
  set(new Builtin_t("%which_test", which_test_bi));
  set(new Builtin_t("%while", while_bi));
  set(new Builtin_t("%var_add", var_add_bi));
  set(new Builtin_t("%var_exists", var_exists_bi));
  set(new Builtin_t("%version", version_bi));
  set(new Builtin_t("%version_available", version_available_bi));
  set(new Builtin_t("%version_compatible", version_compatible_bi));}

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
  Executable_t* i = find(key);
  if (i) {
    argv.push_front(key);
    (*i)(argv);
    argv.pop_front();
    return true;}
  else return false;}

int Executable_map_t::run(Argv_t& argv) {
  Executable_t* i = find(argv);                         // first check for key
  if (i) return (*i)(argv);
  else if (argv[0][0] == '/') {                         // insert a binary
    set(new Binary_t(argv[0]));
    return (*find(argv[0]))(argv);}
  if (!is_argfunction_name(argv[0])) {                  // try autofunction
    run_if_exists("rwsh.autofunction", argv);
    i = find(argv[0]);                                  // second check for key
    if (i) return (*i)(argv);}
  argv.push_front("rwsh.executable_not_found");         // executable_not_found
  i = find(argv[0]);
  if (i) return (*i)(argv);
  set(new Function_t("rwsh.executable_not_found", // reset executable_not_found
                     "%echo $1 : command not found ( $* ); %newline; "
                     "%return -1"));
  return (*find(argv[0]))(argv);}

