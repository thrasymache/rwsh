// The definition of the Arg_spec class which contains a single argument
// specifier (e.g. a fixed string, a variable read, a selection read or $*).
//
// Copyright (C) 2006-2016 Samuel Newbold

#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"
#include "read_dir.cc"
#include "rwshlib.h"
#include "selection.h"
#include "substitution_stream.h"
#include "tokenize.cc"

#include "function.h"
#include "argm_star_var.cc"
#include "selection_read.cc"

Arg_spec::Arg_spec(const std::string& script, unsigned max_soon) :
      soon_level(0), ref_level(0), expand(false), word_selection(-1),
      substitution(0) {
  unsigned i = 1;
  while (i < script.length() && script[i] == '&') ++soon_level, ++i;
  while (i < script.length() && script[i] == '$') ++ref_level, ++i;
  std::string::size_type key_end;
  if (script[0] != '$' && script[0] != '&') key_end = std::string::npos;
  else if (i < script.length()) key_end = script.find('$', i);
  else key_end = i;
  if (key_end < script.length()) {
    expand = true;
    if (key_end + 1 < script.length())
      try {word_selection = my_strtoi(script.substr(key_end+1));}
      catch(...) {
        throw Signal_argm(Argm::Invalid_word_selection,
                          script.substr(key_end));}}
  if (!script.length()) type=FIXED;
  else if (script[0] == '$') {
    if (script.length() < 2) type=REFERENCE;
    else {
      if (script[i] == '*') {
        type=STAR_REF;
        if (script.length() - i > 1) text=script.substr(i+1, key_end-i-1);
        else text="1";}
      else {type=REFERENCE; text=script.substr(i, key_end-i);}}}
  else if (script[0] == '&') {
    if (script.length() < 2) type=SOON;
    else {
      if (soon_level > max_soon)
        throw Signal_argm(Argm::Not_soon_enough, script);
      if (script[i] == '*') {
        type=STAR_SOON;
        if (script.length() - i > 1) text=script.substr(i+1, key_end-i-1);
        else text="1";}
      else {type=SOON; text=script.substr(i, key_end-i);}}}
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

Arg_spec::Arg_spec(const std::string& src, std::string::size_type style_start,
                   std::string::size_type& point, unsigned max_soon) :
      soon_level(0), ref_level(0), expand(false), word_selection(-1),
      substitution(0), text() {
  std::string::size_type tpoint = style_start;
  if (src[tpoint] == '&') {
    type = SOON_SUBSTITUTION;
    while (++tpoint != point && src[tpoint] == '&') ++soon_level;}
  else if (src[tpoint] == '$') {
    type = SUBSTITUTION;
    ++tpoint, soon_level = max_soon;}
  if (src[tpoint] != '{') {
    throw Signal_argm(Argm::Bad_argfunction_style,
                      src.substr(style_start, point-style_start));}
  substitution = new Command_block(src, tpoint, soon_level);
  if (soon_level > max_soon) {
    delete substitution;
    throw Signal_argm(Argm::Not_soon_enough,
                      src.substr(style_start, tpoint-style_start));}
  if (tpoint < src.length()) switch (src[tpoint]) {
    case ' ': case ';': case '\n': case '}': break;
    case '$': {
      expand = true; ++tpoint;
      if (tpoint >= src.length()) break;
      else switch(src[tpoint]) {
        case ' ': case ';': case '\n': case '}': goto after_loop;
        default:
          try {
            std::string::size_type token_end = src.find_first_of(" };", tpoint);
            word_selection = my_strtoi(src.substr(tpoint, token_end-tpoint));
            tpoint = token_end;
            goto after_loop;}
          catch(...) {}}} // fall through
    default: {
      std::string::size_type token_end = src.find_first_of(" };", tpoint);
      throw Signal_argm(Argm::Invalid_word_selection,
                        src.substr(tpoint, token_end-tpoint));}}
  after_loop:
  point = tpoint;}

Arg_spec::Arg_spec(Arg_type type_i, unsigned soon_level_i,
                   unsigned ref_level_i, unsigned expand_i,
                   int word_selection_i, Command_block* substitution_i,
                   const std::string& text_i, const std::string& trailing_i) :
    type(type_i), soon_level(soon_level_i), ref_level(ref_level_i),
    expand(expand_i), word_selection(word_selection_i),
    substitution(substitution_i), text(text_i), trailing(trailing_i) {}

Arg_spec::Arg_spec(const Arg_spec& src) :
  type(src.type), soon_level(src.soon_level), ref_level(src.ref_level),
  expand(src.expand), word_selection(src.word_selection),
  substitution(src.substitution->copy_pointer()), text(src.text),
  trailing(src.trailing) {}

Arg_spec::~Arg_spec() {delete substitution;}

void Arg_spec::apply(const Argm& src, unsigned nesting,
                 std::back_insert_iterator<std::vector<Arg_spec> > res) const {
  switch(type) {
    case SOON:
      if (soon_level)
        *res++ = Arg_spec(type, soon_level-1, ref_level, expand,
                          word_selection, substitution, text, trailing);
      else {
        std::string value = src.get_var(text);
        for (unsigned i = 0; i < ref_level; ++i) value = src.get_var(value);
        *res++ = Arg_spec(FIXED, 0, 0, 0, -1, substitution, value, trailing);}
      break;
    case STAR_SOON:
      if (soon_level)
        *res++ = Arg_spec(type, soon_level-1, ref_level, expand,
                          word_selection, substitution, text, trailing);
      else res = src.star_var(text, ref_level, res);
      break;
    case SUBSTITUTION: case SOON_SUBSTITUTION: {
      Command_block* new_substitution = substitution->apply(src, nesting+1);
      if (soon_level)
        *res++ = Arg_spec(type, soon_level-1, ref_level, expand,
                          word_selection, new_substitution, text, trailing);
      else {
        Substitution_stream override_stream;
        Argm temp_argm(src);
        temp_argm.output = override_stream.child_stream();
        if ((*new_substitution)(temp_argm))
          throw Signal_argm(Argm::Failed_substitution, str());
        *res++ = Arg_spec(FIXED, 0, 0, 0, -1, 0, override_stream.value(),
                          trailing);}
      break;}
    default: *res++ = *this;}}  // most types are not affected by apply

// create a string from Arg_spec. inverse of constructor.
std::string Arg_spec::str(void) const {
  std::string base, result(text);
  if (type != SUBSTITUTION) for (unsigned i=0; i < soon_level; ++i) base += '&';
  for (unsigned i=0; i < ref_level; ++i) base += '$';
  for (std::string::size_type pos = result.find_first_of("\\");
       pos != std::string::npos; pos = result.find_first_of("\\", pos+2))
    result.replace(pos, 1, "\\\\");
  std::string tail;
  if (expand) tail += '$';
  if (word_selection != -1) {
    std::ostringstream tmp;
    tmp <<word_selection;
    tail += tmp.str();}
  switch(type) {
    case FIXED:
      if (!text.length()) return "()" + tail;
      else if (text.find_first_of(" \t\n") != std::string::npos)
        return "(" + result + ")" + tail;
      else if (text.find_last_of("$@()", 1) == 0) return "\\" + result + tail;
      else return result + tail;
    case SOON: return "&" + base + result + tail;
    case REFERENCE: return "$" + base + result + tail;
    case STAR_REF:
      if (text == "1") return base + "$*" + tail;
      else return base + "$*" + result + tail;
    case STAR_SOON:
      if (text == "1") return base + "&*" + tail;
      else return base + "&*" + result + tail;
    case SELECTION: return "@" + result + tail;
    case SELECT_VAR: return "@$" + result + tail;
    case SELECT_STAR_VAR: return "@$*" + result + tail;
    case SOON_SUBSTITUTION: return "&" + base + substitution->str() + tail;
    case SUBSTITUTION: return "$" + base + substitution->str() + tail;
    default: std::abort();}}

// produce one or more strings for destination Argm from Arg_spec and source
// Argm
void Arg_spec::interpret(const Argm& src,
                           std::back_insert_iterator<Argm> res) const {
  switch(type) {
    case FIXED: *res++ = text; break;
    case REFERENCE: case SOON: {
      if (soon_level) std::abort(); // constructor guarantees SOONs already done
      std::string next = src.get_var(text);
      for (unsigned i = 0; i < ref_level; ++i) next = src.get_var(next);
      if (word_selection != -1) {
         std::vector<std::string> intermediate;
         tokenize_words(next, std::back_inserter(intermediate));
         if (word_selection >= intermediate.size())
           throw Signal_argm(Argm::Undefined_variable, str());
         else *res++ = intermediate[word_selection];}
      else if (!expand) *res++ = next;
      else tokenize_words(next, res);
      break;}
    case STAR_REF: case STAR_SOON:
      if (soon_level) std::abort(); // constructor guarantees SOONs already done
      res = src.star_var(text, ref_level, res); break;
    case SELECTION:  selection_read(text, res); break;
    case SELECT_VAR: selection_read(src.get_var(text), res); break;
    case SELECT_STAR_VAR:
      default_output <<"@$* not implemented yet\n"; break;
    case SUBSTITUTION: case SOON_SUBSTITUTION: {
      if (soon_level) std::abort(); // constructor guarantees SOONs already done
      Substitution_stream override_stream;
      Argm temp_argm(src);
      temp_argm.output = override_stream.child_stream();
      if ((*substitution)(temp_argm))
        throw Signal_argm(Argm::Failed_substitution, str());
      if (word_selection != -1) {
         std::vector<std::string> intermediate;
         tokenize_words(override_stream.value(),
                        std::back_inserter(intermediate));
         if (word_selection >= intermediate.size())
           throw Signal_argm(Argm::Undefined_variable, str());
         else *res++ = intermediate[word_selection];}
      else if (expand) tokenize_words (override_stream.value(), res);
      else *res++ = override_stream.value();
      break;}
    default: std::abort();}}

void Arg_spec::promote_soons(unsigned nesting) {
  switch(type) {
    case SOON: case STAR_SOON: soon_level += nesting; break;
    case SUBSTITUTION: case SOON_SUBSTITUTION:
      soon_level += nesting;
      substitution->promote_soons(nesting);
      break;}}

