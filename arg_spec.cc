// The definition of the Arg_spec class which contains a single argument
// specifier (e.g. a fixed string, a variable read, a selection read or $*).
//
// Copyright (C) 2006-2018 Samuel Newbold

#include <cstdlib>
#include <dirent.h>
#include <errno.h>
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
#include "call_stack.h"
#include "executable.h"
#include "executable_map.h"
#include "pipe_stream.h"
#include "prototype.h"
#include "read_dir.cc"
#include "rwshlib.h"
#include "selection.h"
#include "substitution_stream.h"
#include "tokenize.cc"

#include "function.h"
#include "argm_star_var.cc"
#include "selection_read.cc"

namespace {
bool sub_term_char(char focus) {
  switch (focus) {
    case ' ': case ';': case '\t': case '\n': case '}': return true;
    default: return false;}}
} // close unnamed namespace

// Everything without braces
Arg_spec::Arg_spec(const std::string& script, unsigned max_soon,
                   Error_list& errors) :
      soon_level(0), ref_level(0), expand_count(0), word_selection(-1),
      substitution(0) {
  std::string::size_type key_start, key_end;
  if (script[0] == '$' || script[0] == '&') {
    unsigned i = 1;
    while (i < script.length() && script[i] == '&') ++soon_level, ++i;
    while (i < script.length() && script[i] == '$') ++ref_level, ++i;
    key_start = i;
    key_end = script.find('$', i);
    for (i=key_end; i<script.length() && script[i] == '$'; ++i) ++expand_count;
    if (i < script.length())
      try {word_selection = my_strtoi(script.substr(i));}
      catch(...) {
        errors.add_error(Exception(Argm::Invalid_word_selection,
                                   script.substr(key_end)));}}
  if (!script.length()) type=FIXED;
  else if (script[0] == '$')
    if (script.length() < 2) type=REFERENCE;
    else
      if (script[key_start] == '*') {
        type = STAR_REF;
        if (script.length() - key_start > 1)
          text = script.substr(key_start+1, key_end-key_start-1);
        else text="1";}
      else {type=REFERENCE; text=script.substr(key_start, key_end-key_start);}
  else if (script[0] == '&') {
    type=SOON;
    if (script.length() >= 2)
      if (soon_level > max_soon)
        errors.add_error(Exception(Argm::Not_soon_enough, script));
      else if (script[key_start] == '*') {
        type=STAR_SOON;
        if (script.length() - key_start > 1)
          text=script.substr(key_start+1, key_end-key_start-1);
        else text="1";}
      else text=script.substr(key_start, key_end-key_start);
    else;}
  else if (script[0] == '@')
    if (script.length() < 2) type=SELECTION;
    else if (script[1] == '$')
      if (script.length() < 3) type=SELECT_VAR;
      else if (script[2] == '*') {
        type=SELECT_STAR_VAR;
        if (script.length() > 3) text=script.substr(3);
        else text="1";}
      else {type=SELECT_VAR; text=script.substr(2);}
    else {type=SELECTION; text=script.substr(1);}
  else if (script[0] == '\\') {type=FIXED; text=script.substr(1);}
  else {type=FIXED; text=script;}};

void Arg_spec::parse_brace_type(const std::string& src, unsigned max_soon,
                  std::string::size_type style, std::string::size_type end,
                  Error_list& errors) {
  auto focus = style;
  if (src[focus] == '.') {
    type = FIXED;
    ++focus;}
  else if (src[focus] == '[' && src[focus+1] == '.') {
    type = FIXED;
    focus += 2;}
  else if (src[focus] == '&') {
    type = SOON_SUBSTITUTION;
    while (++focus < end && src[focus] == '&') ++soon_level;}
  else if (src[focus] == '$') {
    type = SUBSTITUTION;
    ++focus, soon_level = max_soon;}
  else type = FIXED; // This is a Bad_argfunction_style, but since we're going
  // to continue processing, we need to properly initialize the structure
  if (focus != end)
    errors.add_error(Exception(Argm::Bad_argfunction_style,
                      src.substr(style, end-style)));}

// Substitutions, soon substitutions, and literal brace strings
Arg_spec::Arg_spec(const std::string& src, std::string::size_type style,
        std::string::size_type& point, unsigned max_soon, Error_list& errors) :
      soon_level(0), ref_level(0), expand_count(0), word_selection(-1),
      substitution(0), text() {
  parse_brace_type(src, max_soon, style, point, errors);
  if (type == FIXED) {
    auto split = src.find_first_of('}', point);
    if (split == std::string::npos)
      throw Unclosed_brace(src.substr(0, point-1));
    if (src[style] == '[')
      if (src[split+1] == ']') ++split;
      else errors.add_error(Exception(Argm::Mismatched_bracket,
                      src.substr(style, split-style+1)));
    else;
    text = src.substr(style, split-style+1);
    point = split+1;}
  else {
    substitution = new Command_block(src, point, soon_level, errors);
    if (point < src.length() && !sub_term_char(src[point]))
      parse_word_selection(src, point, errors);
    if (soon_level > max_soon)
      errors.add_error(Exception(Argm::Not_soon_enough,
                                 src.substr(style, point-style)));}}

void Arg_spec::parse_word_selection(const std::string& src,
                          std::string::size_type& point, Error_list& errors){
  auto token_end = src.find_first_of(" };", point);
  if (src[point] == '$') {
    do ++expand_count;
    while (++point < src.length() && src[point] == '$');
    if (point >= src.length() || sub_term_char(src[point]));
    else
      try {word_selection = my_strtoi(src.substr(point, token_end-point));}
      catch(...) {
        errors.add_error(Exception(Argm::Invalid_word_selection,
                                   src.substr(point, token_end-point)));}}
  else errors.add_error(Exception(Argm::Invalid_word_selection,
                                  src.substr(point, token_end-point)));
  point = token_end;}

Arg_spec::Arg_spec(Arg_type type_i, unsigned soon_level_i,
                   unsigned ref_level_i, unsigned expand_count_i,
                   int word_selection_i, Command_block* substitution_i,
                   const std::string& text_i, const std::string& trailing_i) :
    type(type_i), soon_level(soon_level_i), ref_level(ref_level_i),
    expand_count(expand_count_i), word_selection(word_selection_i),
    substitution(substitution_i), text(text_i), trailing(trailing_i) {}

Arg_spec::Arg_spec(const Arg_spec& src) :
  type(src.type), soon_level(src.soon_level), ref_level(src.ref_level),
  expand_count(src.expand_count), word_selection(src.word_selection),
  substitution(src.substitution->copy_pointer()), text(src.text),
  trailing(src.trailing) {}

Arg_spec::Arg_spec(const std::string& src) :
    type(FIXED), soon_level(0), ref_level(0), expand_count(0),
    word_selection(-1), substitution(0), text(src), trailing(" ") {}

Arg_spec::~Arg_spec() {delete substitution;}

void Arg_spec::apply(const Argm& src, unsigned nesting,
                     std::back_insert_iterator<std::vector<Arg_spec> > res,
                     Error_list& exceptions) const {
  switch(type) {
    case SOON:
      if (soon_level)
        *res++ = Arg_spec(type, soon_level-1, ref_level, expand_count,
                          word_selection, substitution, text, trailing);
      else res = evaluate_var(src, res);
      break;
    case STAR_SOON:
      if (soon_level)
        *res++ = Arg_spec(type, soon_level-1, ref_level, expand_count,
                          word_selection, substitution, text, trailing);
      else res = src.star_var(text, ref_level, res);
      break;
    case SUBSTITUTION: case SOON_SUBSTITUTION:
      if (soon_level)
        *res++ = Arg_spec(type, soon_level-1, ref_level, expand_count,
                          word_selection,
                          substitution->apply(src, nesting+1, exceptions),
                          text, trailing);
      else evaluate_substitution(src, res, exceptions);
      break;
    default: *res++ = *this;}}  // most types are not affected by apply

template<class Out>
Out Arg_spec::evaluate_expansion(const std::string& value, Out res)
    const {
  if (!expand_count) *res++ = value;
  else {
     std::vector<std::string> intermediate(1, value);
     for (unsigned i=(word_selection == -1); i<expand_count; ++i) {
       std::vector<std::string> temp(intermediate);
       intermediate.clear();
       for (unsigned j=0; j<temp.size(); ++j)
         tokenize_words(temp[j], std::back_inserter(intermediate));}
     if (word_selection == -1)
       for (unsigned j=0; j<intermediate.size(); ++j)
         tokenize_words(intermediate[j], res);
     else if (word_selection >= (int)intermediate.size())
       throw Exception(Argm::Undefined_variable, str());
     else *res++ = intermediate[word_selection];}
  return res;}

template<class Out>
Out Arg_spec::evaluate_substitution(const Argm& src, Out res,
                                    Error_list& exceptions) const {
  Substitution_stream override_stream;
  Argm temp_argm(src);
  temp_argm.output = override_stream.child_stream();
  (*substitution)(temp_argm, exceptions);
  if (global_stack.unwind_stack())
      exceptions.add_error(Exception(Argm::Failed_substitution, str()));
  else return evaluate_expansion(override_stream.value(), res);
  return res;}

template<class Out> Out Arg_spec::evaluate_var(const Argm& src, Out res) const {
  std::string focus = text;
  try {
    for (unsigned i = 0; i <= ref_level; ++i) focus = src.get_var(focus);
    return evaluate_expansion(focus, res);}
  catch (Undefined_variable) {
    if (!expand_count) throw;      // $var throws $var$ does not
    src.var_exists(focus);         // this counts as a variable check
    return res;}}

// produce one or more strings for destination Argm from Arg_spec and source
// Argm
void Arg_spec::interpret(const Argm& src,
                         std::back_insert_iterator<Argm> res,
                         Error_list& exceptions) const {
  if (soon_level) std::abort();    // constructor guarantees SOONs already done
  switch(type) {
    case FIXED: *res++ = text; break;
    case REFERENCE: case SOON: evaluate_var(src, res); break;
    case STAR_REF: case STAR_SOON:
      res = src.star_var(text, ref_level, res); break;
    case SELECTION:  selection_read(text, res); break;
    case SELECT_VAR: selection_read(src.get_var(text), res); break;
    case SELECT_STAR_VAR: default_output <<"@$* not implemented yet\n"; break;
    case SUBSTITUTION: case SOON_SUBSTITUTION:
      evaluate_substitution(src, res, exceptions); break;
    default: std::abort();}}

void Arg_spec::promote_soons(unsigned nesting) {
  switch(type) {
    case SOON: case STAR_SOON: soon_level += nesting; break;
    case SUBSTITUTION: case SOON_SUBSTITUTION:
      soon_level += nesting;
      substitution->promote_soons(nesting);
      break;
    case FIXED: case REFERENCE: case SELECTION: case SELECT_VAR:
    case SELECT_STAR_VAR: case STAR_REF:;}}                     // not relevant

// create a string from Arg_spec. inverse of constructor.
std::string Arg_spec::str(void) const {
  std::string base, result(text);
  if (type != SUBSTITUTION) for (unsigned i=0; i < soon_level; ++i) base += '&';
  for (unsigned i=0; i < ref_level; ++i) base += '$';
  for (auto pos = result.find_first_of("\\"); pos != std::string::npos;
       pos = result.find_first_of("\\", pos+2))
    result.replace(pos, 1, "\\\\");
  std::string tail;
  for (unsigned i=0; i < expand_count; ++i) tail += '$';
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
