// The definition of the Arg_spec_t class which contains a single argument
// specifier (e.g. a fixed string, a variable read, a selection read or $*).
// The definition of the Arg_script_t class which contains a vector of 
// Arg_spec_t objects and a Function_t*, and which represents the mapping 
// between the Argv_t that was passed to the function and the Argv_t that will 
// be used to run a given executable.
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <iterator>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <vector>

#include "argv.h"
#include "argv_star_var.cc"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "read_dir.cc"
#include "rwsh_stream.h"
#include "selection.h"
#include "selection_read.cc"
#include "substitution_stream.h"
#include "util.h"
#include "variable_map.h"

Arg_spec_t::Arg_spec_t(const std::string& script, unsigned max_soon) : 
      soon_level(0), ref_level(0), substitution(0) {
  if (!script.length()) type=FIXED;
  else if (script[0] == '$') {
    if (script.length() < 2) type=REFERENCE;
    else {
      int i = 1;
      while (i < script.length() && script[i] == '$') ++ref_level, ++i;
      if (script[i] == '*') {
        type=STAR_REF;
        if (script.length() - i > 1) text=script.substr(i+1);
        else text="1";}
      else {type=REFERENCE; text=script.substr(i);}}}
  else if (script[0] == '&') {
    if (script.length() < 2) type=SOON;
    else {
      unsigned i = 1;
      while (i < script.length() && script[i] == '&') ++soon_level, ++i;
      while (i < script.length() && script[i] == '$') ++ref_level, ++i;
      if (soon_level > max_soon) throw Not_soon_enough_t(script);
      type=SOON; text=script.substr(i);}}
  else if (script[0] == '@') {
    if (script.length() < 2) type=SELECTION;
    else if (script[1] == '$') {
      if (script.length() < 3) type=SELECT_VAR;
      else if (script[2] == '*') {
        type=SELECT_STAR_VAR;
        if (script.length() > 3) text=script.substr(3);
        else text="1";}
      else {type=SELECT_VAR; text=script.substr(2);}}
    else {type=SELECTION; text=script.substr(1);}}
  else if (script[0] == '\\') {type=FIXED; text=script.substr(1);}
  else {type=FIXED; text=script;}};

Arg_spec_t::Arg_spec_t(const std::string& style, const std::string& function, 
                       unsigned max_soon) : 
      type(SUBSTITUTION), soon_level(0), ref_level(0), substitution(0), text() {
  unsigned i = 1;
  while (i < style.length() && style[i] == '&') ++soon_level, ++i;
  if (i != style.length()) {
    throw Bad_argfunction_style_t(style);}
  if (soon_level > max_soon) 
    throw Not_soon_enough_t(style + "{" + function + "}");
  substitution = new Function_t("rwsh.argfunction", function, soon_level);}

Arg_spec_t::Arg_spec_t(const Arg_spec_t& src) : 
  type(src.type), soon_level(src.soon_level), ref_level(src.ref_level), 
  substitution(src.substitution->copy_pointer()), text(src.text) {}

Arg_spec_t::~Arg_spec_t() {delete substitution;}

void Arg_spec_t::apply(const Argv_t& src) {
  switch(type) {
    case SOON: 
      if (soon_level) --soon_level;
      else {
        text = src.get_var(text);
        for (unsigned i = 0; i < ref_level; ++i) text = src.get_var(text);
        type = FIXED;}
      break;
    case SUBSTITUTION: {
      Function_t* temp = substitution->apply(src);
      delete substitution;
      substitution = temp;
      if (soon_level) --soon_level;
      else {
        Substitution_stream_t override_stream;
        if ((*substitution)(src, &override_stream)) 
          throw Failed_substitution_t(str());
        delete substitution;
        substitution = 0;
        text = override_stream.str();
        type = FIXED;}
      break;}
    default: break;}}  // most types are not affected by apply

// create a string from Arg_spec. inverse of constructor.
std::string Arg_spec_t::str(void) const {
  std::string base;
  for (int i=0; i < soon_level; ++i) base += '&';
  for (int i=0; i < ref_level; ++i) base += '$';
  switch(type) {
    case FIXED: 
      if (!text.length()) return "\\ ";
      else if(text[0] == '$' || text[0] == '@' || text[0] == '\\')
        return "\\" + text;
      else return text;
    case SOON: return "&" + base + text;
    case REFERENCE: return "$" + base + text;
    case STAR_REF: 
      if (text == "1") return base + "$*";
      else return base + "$*" + text;
    case SELECTION: return "@" + text;
    case SELECT_VAR: return "@$" + text;
    case SELECT_STAR_VAR: return "@$*" + text;
    case SUBSTITUTION: return "&" + base + substitution->str();
    default: assert(0);}}

// produce one or more strings for destination Argv from Arg_spec and source
// Argv
template<class Out>
void Arg_spec_t::interpret(const Argv_t& src, Out res) const {
  switch(type) {
    case FIXED:      *res++ = text; break;
    case REFERENCE: case SOON: {
      assert(!soon_level); // constructor guarantees SOONs are already done
      std::string next = src.get_var(text);
      for (unsigned i = 0; i < ref_level; ++i) next = src.get_var(next);
      *res++ = next;
      break;}
    case STAR_REF:   res = src.star_var(text, ref_level, res); break;
    case SELECTION:  selection_read(text, res); break;
    case SELECT_VAR: selection_read(src.get_var(text), res); break;
    case SELECT_STAR_VAR: 
      *default_stream_p <<"@$* not implemented yet\n"; break;
    case SUBSTITUTION: {
      assert(!soon_level); // constructor guarantees SOONs are already done
      Substitution_stream_t override_stream;
      if ((*substitution)(src, &override_stream)) 
        throw Failed_substitution_t(str()); 
      *res++ = override_stream.str();
      break;}
    default: assert(0);}}

Arguments_to_argfunction_t::Arguments_to_argfunction_t(
      const std::string& argfunction_type) : Argv_t(default_stream_p) {
  push_back("rwsh.arguments_for_argfunction");
  push_back(argfunction_type);}

Bad_argfunction_style_t::Bad_argfunction_style_t(
      const std::string& argfunction_style) : Argv_t(default_stream_p) {
  push_back("rwsh.bad_argfunction_style");
  push_back(argfunction_style);}

Failed_substitution_t::Failed_substitution_t(const std::string& function) :
      Argv_t(default_stream_p) {
  push_back("rwsh.failed_substitution");
  push_back(function);}

Mismatched_brace_t::Mismatched_brace_t(const std::string& prefix) : 
      Argv_t(default_stream_p) {
  push_back("rwsh.mismatched_brace");
  push_back(prefix);}

Multiple_argfunctions_t::Multiple_argfunctions_t() : Argv_t(default_stream_p) {
  push_back("rwsh.multiple_argfunctions");}

Not_soon_enough_t::Not_soon_enough_t(const std::string& argument) : 
      Argv_t(default_stream_p) {
  push_back("rwsh.not_soon_enough");
  push_back(argument);}

Undefined_variable_t::Undefined_variable_t(const std::string& variable) :
      Argv_t(default_stream_p) {
  push_back("rwsh.undefined_variable");
  push_back(variable);}
  
Arg_script_t::Arg_script_t(const std::string& src, unsigned max_soon) :
      argfunction(0), argfunction_level(0), myout(default_stream_p) {
  std::string::size_type token_start = src.find_first_not_of(" "); 
  while (token_start != std::string::npos) {
    std::string::size_type token_end = src.find_first_of("{} ", token_start); 
    if (token_end == std::string::npos) 
      push_back(Arg_spec_t(src.substr(token_start, std::string::npos), 
                           max_soon));
    else switch (src[token_end]) {                           // add one argument
      case ' ': 
        push_back(Arg_spec_t(src.substr(token_start, token_end-token_start),
                             max_soon)); 
        break;
      case '{': {
        std::string::size_type close_brace = find_close_brace(src, token_end);
        if (close_brace == std::string::npos)
          throw Mismatched_brace_t(src.substr(0, token_end+1));
        std::string f_str = src.substr(token_end+1, close_brace-token_end-1);
        if (token_start != token_end) {
          std::string style = src.substr(token_start, token_end-token_start);
          push_back(Arg_spec_t(style, f_str, max_soon));}
        else {
          if (argfunction) throw Multiple_argfunctions_t();
          argfunction = new Function_t("rwsh.argfunction", f_str, max_soon+1);}
        token_end = close_brace+1;
        break;}
      case '}': throw Mismatched_brace_t(src.substr(0, token_end+1));
      default: assert(0);} // error in std::string::find_first_of
    token_start = src.find_first_not_of(" ", token_end);}
  if (!size()) push_back(Arg_spec_t("", max_soon));
  if (is_argfunction_name(front().str()) &&        // argfunction_level handling
      front().str() != "rwsh.mapped_argfunction") {
    if (size() != 1 || argfunction) 
      throw Arguments_to_argfunction_t(front().str());
    if (front().str() == "rwsh.unescaped_argfunction") argfunction_level = 1;
    else if (front().str() == "rwsh.argfunction") argfunction_level = 2;
    else if (front().str() == "rwsh.escaped_argfunction") argfunction_level = 3;
    else assert(0);}}                            // unhandled argfunction level

Arg_script_t::Arg_script_t(const Arg_script_t& src) : 
  Base(src), argfunction(src.argfunction->copy_pointer()),
 argfunction_level(src.argfunction_level), myout(src.myout) {}

Arg_script_t& Arg_script_t::operator=(const Arg_script_t& src) {
  this->clear();
  copy(src.begin(), src.end(), std::back_inserter(*this));
  argfunction = src.argfunction->copy_pointer();
  argfunction_level = src.argfunction_level;
  myout = src.myout;}

Arg_script_t::~Arg_script_t(void) {delete argfunction;}

// naively create an Argv_t from Arg_script. string constructor for Argv_t.
Argv_t Arg_script_t::argv(void) const {
  Argv_t result(myout);
  if (!argfunction_level) {
    for(const_iterator i=begin(); i != end(); ++i) result.push_back(i->str());
    result.set_argfunction(argfunction->copy_pointer());}
  else if (argfunction_level == 1) result.push_back("rwsh.argfunction");
  else if (argfunction_level == 2) result.push_back("rwsh.escaped_argfunction");
  else assert(0);
  return result;} // unhandled argfunction_level

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
  Argv_t result(myout);
  if (myout == default_stream_p && src.myout() != default_stream_p)
    result.set_myout(src.myout());
  if (!argfunction_level) {
    for (const_iterator i = begin(); i != end(); ++i) 
      i->interpret(src, std::back_inserter(result));
    if (!result.size()) result.push_back("");
    if (argfunction) result.set_argfunction(argfunction->apply(src));}
  else if (is_argfunction()) {
    result.push_back("rwsh.mapped_argfunction");
    copy(src.begin()+1, src.end(), std::back_inserter(result));
    result.set_argfunction(src.argfunction()->copy_pointer());}
  else if (argfunction_level == 2) 
    result.push_back("rwsh.unescaped_argfunction");
  else if (argfunction_level == 3) result.push_back("rwsh.argfunction");
  else assert(0); // unhandled argfunction_level
  return result;}

// produce a new Arg_script by unescaping argument functions and replacing
// unescaped_argfunction with argv.argfunction
Arg_script_t Arg_script_t::apply(const Argv_t& src) const {
  Arg_script_t result(*this);
  if (result.argfunction_level) --result.argfunction_level;  
  else {
    for (iterator i = result.begin(); i != result.end(); ++i) i->apply(src);
    if (this->argfunction) result.argfunction = this->argfunction->apply(src);}
  return result;}

void Arg_script_t::clear(void) {
  delete argfunction; 
  argfunction = 0;
  myout=0; 
  Base::clear();}

// test whether an executable name corresponds to one of those used for
// argument functions.
bool is_argfunction_name(const std::string& focus) {
  return focus == "rwsh.mapped_argfunction" || 
         focus == "rwsh.unescaped_argfunction" ||
         focus == "rwsh.argfunction" ||
         focus == "rwsh.escaped_argfunction";}

// test whether an executable name corresponds to a binary executable
// (i.e. filesystem path)
bool is_binary_name(const std::string& focus) {
  return !focus.empty() && focus[0] == '/';}

// test whether an executable name is possible for a builtin, does not test 
// whether this builtin exists
bool is_builtin_name(const std::string& focus) {
  return !focus.empty() && focus[0] == '%';}

// test whether an executable name is possible for a function
bool is_function_name(const std::string& focus) {
  return !is_binary_name(focus) && !is_builtin_name(focus);}

