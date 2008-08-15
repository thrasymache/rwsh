// The definition of the Arg_spec_t class which contains a single argument
// specifier (e.g. a fixed string, a variable read, a selection read or $*).
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "function.h"
#include "read_dir.cc"
#include "selection.h"
#include "substitution_stream.h"
#include "tokenize.cc"
#include "variable_map.h"

#include "argv_star_var.cc"
#include "selection_read.cc"

Arg_spec_t::Arg_spec_t(const std::string& script, unsigned max_soon) : 
      soon_level(0), ref_level(0), expand_count(0), substitution(0) {
  std::string::size_type key_end = script.find_last_not_of('$');
  if (key_end != std::string::npos)
    expand_count = script.substr(key_end).size() - 1;
  if (!script.length()) type=FIXED;
  else if (script[0] == '$') {
    if (script.length() < 2) type=REFERENCE;
    else {
      unsigned i = 1;
      while (i < script.length() && script[i] == '$') ++ref_level, ++i;
      if (script[i] == '*') {
        type=STAR_REF;
        if (script.length() - i > 1) text=script.substr(i+1, key_end-i);
        else text="1";}
      else {type=REFERENCE; text=script.substr(i, key_end-i+1);}}}
  else if (script[0] == '&') {
    if (script.length() < 2) type=SOON;
    else {
      unsigned i = 1;
      while (i < script.length() && script[i] == '&') ++soon_level, ++i;
      while (i < script.length() && script[i] == '$') ++ref_level, ++i;
      if (soon_level > max_soon) throw Not_soon_enough_t(script);
      if (script[i] == '*') {
        type=STAR_SOON;
        if (script.length() - i > 1) text=script.substr(i+1, key_end-i);
        else text="1";}
      else {type=SOON; text=script.substr(i, key_end-i+1);}}}
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

Arg_spec_t::Arg_spec_t(const std::string& src,
                       std::string::size_type style_start, 
                       std::string::size_type& point, unsigned max_soon) : 
      type(SUBSTITUTION), soon_level(0), ref_level(0), expand_count(0),
      substitution(0), text() {
  std::string::size_type tpoint = style_start+1;
  while (tpoint != point && src[tpoint] == '&') ++soon_level, ++tpoint;
  substitution = new Function_t("rwsh.argfunction", src, tpoint, soon_level);
  if (soon_level+1 != point-style_start || src[style_start] != '&') {
    delete substitution;
    throw Bad_argfunction_style_t(src.substr(style_start, tpoint-style_start));}
  if (soon_level > max_soon) {
    delete substitution;
    throw Not_soon_enough_t(src.substr(style_start, tpoint-style_start));}
  point = tpoint;}

Arg_spec_t::Arg_spec_t(Arg_type_t type_i, unsigned soon_level_i,
                       unsigned ref_level_i, unsigned expand_count_i,
                       Function_t* substitution_i, std::string text_i) :
    type(type_i), soon_level(soon_level_i), ref_level(ref_level_i),
    expand_count(expand_count_i), substitution(substitution_i), text(text_i) {}

Arg_spec_t::Arg_spec_t(const Arg_spec_t& src) : 
  type(src.type), soon_level(src.soon_level), ref_level(src.ref_level), 
  expand_count(src.expand_count),
  substitution(src.substitution->copy_pointer()), text(src.text) {}

Arg_spec_t::~Arg_spec_t() {delete substitution;}

void Arg_spec_t::apply(const Argv_t& src, unsigned nesting,
                std::back_insert_iterator<std::vector<Arg_spec_t> > res) const {
  switch(type) {
    case SOON: 
      if (soon_level)
        *res++ = Arg_spec_t(type, soon_level-1, ref_level, expand_count,
                            substitution, text);
      else {
        std::string new_text = src.get_var(text);
        for (unsigned i = 0; i < ref_level; ++i)
          new_text = src.get_var(new_text);
        *res++ = Arg_spec_t(FIXED, 0, 0, 0, substitution, new_text);}
      break;
    case STAR_SOON: 
      if (soon_level)
        *res++ = Arg_spec_t(type, soon_level-1, ref_level, expand_count,
                            substitution, text);
      else res = src.star_var(text, ref_level, res);
      break;
    case SUBSTITUTION: {
      Function_t* new_substitution = substitution->apply(src, nesting+1);
      if (soon_level) *res++ = Arg_spec_t(type, soon_level-1, ref_level,
                                          expand_count, new_substitution, text);
      else {
        Substitution_stream_t override_stream;
        Argv_t temp_argv(src);
        temp_argv.output = override_stream.child_stream();
        if ((*new_substitution)(temp_argv)) throw Failed_substitution_t(str());
        *res++ = Arg_spec_t(FIXED, 0, 0, 0, 0, override_stream.value());}
      break;}
    default: *res++ = *this;}}  // most types are not affected by apply

// create a string from Arg_spec. inverse of constructor.
std::string Arg_spec_t::str(void) const {
  std::string base;
  for (unsigned i=0; i < soon_level; ++i) base += '&';
  for (unsigned i=0; i < ref_level; ++i) base += '$';
  std::string tail;
  for (unsigned i=0; i < expand_count; ++i) tail += '$';
  switch(type) {
    case FIXED: 
      if (!text.length()) return "()";
      switch(text[0]) {
        case '$': case '@': case '\\': case ' ': return "\\" + text;
        default: return text;}
    case SOON: return "&" + base + text + tail;
    case REFERENCE: return "$" + base + text + tail;
    case STAR_REF: 
      if (text == "1") return base + "$*" + tail;
      else return base + "$*" + text + tail;
    case STAR_SOON: 
      if (text == "1") return base + "&*" + tail;
      else return base + "&*" + text + tail;
    case SELECTION: return "@" + text;
    case SELECT_VAR: return "@$" + text;
    case SELECT_STAR_VAR: return "@$*" + text;
    case SUBSTITUTION: return "&" + base + substitution->str() + tail;
    default: assert(0);}}

// produce one or more strings for destination Argv from Arg_spec and source
// Argv
void Arg_spec_t::interpret(const Argv_t& src,
                           std::back_insert_iterator<Argv_t> res) const {
  switch(type) {
    case FIXED:      *res++ = text; break;
    case REFERENCE: case SOON: {
      assert(!soon_level); // constructor guarantees SOONs are already done
      std::string next = src.get_var(text);
      for (unsigned i = 0; i < ref_level; ++i) next = src.get_var(next);
      if (!expand_count) *res++ = next;
      else tokenize(next, res, std::bind2nd(std::equal_to<char>(), ' '));
      break;}
    case STAR_REF: case STAR_SOON:
      assert(!soon_level); // constructor guarantees SOONs are already done
      res = src.star_var(text, ref_level, res); break;
    case SELECTION:  selection_read(text, res); break;
    case SELECT_VAR: selection_read(src.get_var(text), res); break;
    case SELECT_STAR_VAR: 
      default_output <<"@$* not implemented yet\n"; break;
    case SUBSTITUTION: {
      assert(!soon_level); // constructor guarantees SOONs are already done
      Substitution_stream_t override_stream;
      Argv_t temp_argv(src);
      temp_argv.output = override_stream.child_stream();
      if ((*substitution)(temp_argv)) 
        throw Failed_substitution_t(str()); 
      *res++ = override_stream.value();
      break;}
    default: assert(0);}}

void Arg_spec_t::promote_soons(unsigned nesting) {
  switch(type) {
    case SOON: case STAR_SOON: soon_level += nesting; break;
    case SUBSTITUTION:
      soon_level += nesting;
      Function_t* temp = substitution->promote_soons(nesting);
      delete substitution;
      substitution = temp; break;}}

