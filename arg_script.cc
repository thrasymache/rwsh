// The definition of the Arg_spec_t class which contains a single argument
// specifier (e.g. a fixed string, a variable read, a selection read or $*).
// The definition of the Arg_script_t class which contains a vector of 
// Arg_spec_t objects and a Function_t*, and which represents the mapping 
// between the Argv_t that was passed to the function and the Argv_t that will 
// be used to run a given executable.
//
// Copyright (C) 2006 Samuel Newbold

#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "argv.h"
#include "argv_star_var.cc"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "read_dir.cc"
#include "selection.h"
#include "selection_read.cc"
#include "variable_map.h"

Arg_spec_t::Arg_spec_t(const std::string& script) {
  if (!script.length()) type=FIXED;
  else if (script[0] == '$') {
    if (script.length() < 2) type=VARIABLE;
    else if (script[1] == '*') {
      type=STAR_VAR;
      if (script.length() > 2) text=script.substr(2);
      else text="1";}
    else if (isargvar(script.substr(1))) {type=ARG_VAR; text=script.substr(1);}
    else {type=VARIABLE; text=script.substr(1);}}
  else if (script[0] == '@') {
    if (script.length() < 2) type=SELECTION;
    else if (script[1] == '$') {
      if (script.length() < 3) type=SELECT_VAR;
      else if (script[2] == '*') {
        type=SELECT_STAR_VAR;
        if (script.length() > 3) text=script.substr(3);
        else text="0";}
      else if (isargvar(script.substr(2))) {
        type=SELECT_ARG_VAR; text=script.substr(2);}
      else {type=SELECT_VAR; text=script.substr(2);}}
    else {type=SELECTION; text=script.substr(1);}}
  else if (script[0] == '\\') {type=FIXED; text=script.substr(1);}
  else {type=FIXED; text=script;}};

// create a string from Arg_spec. inverse of constructor.
std::string Arg_spec_t::str(void) const {
  switch(type) {
    case FIXED: 
      if (!text.length() || text[0] == '$' || text[0] == '@' || text[0] == '\\')
        return "\\" + text;
      else return text;
    case VARIABLE: return "$" + text;
    case ARG_VAR: return "$" + text;
    case STAR_VAR: 
      if (text == "1") return "$*";
      else return "$*" + text;
    case SELECTION: return "@" + text;
    case SELECT_VAR: return "@$" + text;
    case SELECT_ARG_VAR: return "@$" + text;
    case SELECT_STAR_VAR: return "@$*" + text;}}

// produce one or more strings for destination Argv from Arg_spec and source
// Argv
template<class Out>
void Arg_spec_t::interpret(const Argv_t& src, Out res) const {
  switch(type) {
    case FIXED:      *res++ = text; break;
    case VARIABLE:   *res++ = get_var(text); break;
    case ARG_VAR:    *res++ = src.get_var(text); break;
    case STAR_VAR:   res = src.star_var(text, res); break;
    case SELECTION:  selection_read(text, res); break;
    case SELECT_VAR: selection_read(get_var(text), res); break;
    case SELECT_ARG_VAR: selection_read(src.get_var(text), res); break;
    case SELECT_STAR_VAR: std::cerr <<"@$* not implemented yet\n"; break;}}

Arg_spec_t& construct_arg_spec(const std::string& src) {
  return *(new Arg_spec_t(src));}

Arg_script_t::Arg_script_t(const Argv_t& src) 
  throw (Arguments_to_argfunction_t): 
  argfunction_level(0), argfunction(src.argfunction()->copy_pointer()) {
  if (!src.size()) this->push_back(Arg_spec_t(""));
  else if (is_argfunction_name(src[0]) && src[0] != "rwsh.mapped_argfunction") {
    if (src.size() != 1 || src.argfunction()) {
      std::string error_string = "rwsh.arguments_for_argfunction ";
      error_string += src[0];
      throw Arguments_to_argfunction_t(error_string);}
    if (src[0] == "rwsh.unescaped_argfunction") argfunction_level = 1;
    else if (src[0] == "rwsh.argfunction") argfunction_level = 2;
    else if (src[0] == "rwsh.escaped_argfunction") argfunction_level = 3;}
  else transform(src.begin(), src.end(), std::back_inserter(*this), 
                 construct_arg_spec);}

Arg_script_t::Arg_script_t(const Arg_script_t& src) : 
  Base(src), argfunction_level(src.argfunction_level), 
  argfunction(src.argfunction->copy_pointer()) {}

// produce a new Arg_script by unescaping argument functions and replacing
// unescaped_argfunction with argv.argfunction
Arg_script_t::Arg_script_t(const Arg_script_t& src, const Argv_t& argv) :
  Base(src), 
  argfunction_level(src.argfunction_level? src.argfunction_level-1: 0), 
  argfunction(0) {
  if (src.argfunction) argfunction = src.argfunction->interpret(argv);}

Arg_script_t& Arg_script_t::operator=(const Arg_script_t& src) {
  this->clear();
  argfunction_level = src.argfunction_level;
  copy(src.begin(), src.end(), std::back_inserter(*this));
  argfunction = src.argfunction->copy_pointer();}

Arg_script_t::~Arg_script_t(void) {delete argfunction;}

// create a string from Arg_script. inverse of string constructor.
std::string Arg_script_t::str(void) const {
  if (!argfunction_level) {
    std::string result;
    for(const_iterator i=begin(); i != end()-1; ++i) result += i->str() + ' ';
    result += back().str();
    if (argfunction) result += " " + argfunction->str();
    return result;}
  else if (argfunction_level == 1) return "rwsh.argfunction";
  else if (argfunction_level == 2) return "rwsh.escaped_argfunction";
  else {assert(0); return "";}} // unhandled argfunction_level

// produce a destination Argv from the source Argv according to this script
Argv_t Arg_script_t::interpret(const Argv_t& src) const {
  Argv_t result;
  if (!argfunction_level) {
    for (const_iterator i = begin(); i != end(); ++i) 
      i->interpret(src, std::back_inserter(result));
    if (argfunction) result.set_argfunction(argfunction->interpret(src));}
  else if (is_argfunction()) {
    result.push_back("rwsh.mapped_argfunction");
    copy(src.begin()+1, src.end(), std::back_inserter(result));
    result.set_argfunction(src.argfunction()->copy_pointer());}
  else if (argfunction_level == 2) 
    result.push_back("rwsh.unescaped_argfunction");
  else if (argfunction_level == 3) result.push_back("rwsh.argfunction");
  else assert(0); // unhandled argfunction_level
  return result;}

void Arg_script_t::clear(void) {delete argfunction; Base::clear();}

// test for whether an executable name corresponds to one of those used for
// argument functions.
bool is_argfunction_name(const std::string& focus) {
  return focus == "rwsh.mapped_argfunction" || 
         focus == "rwsh.unescaped_argfunction" ||
         focus == "rwsh.argfunction" ||
         focus == "rwsh.escaped_argfunction";}

