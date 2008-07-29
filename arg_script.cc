// The definition of the Arg_script_t class which contains a vector of 
// Arg_spec_t objects and a Function_t*, and which represents the mapping 
// between the Argv_t that was passed to the function and the Argv_t that will 
// be used to run a given executable.
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <assert.h>
#include <exception>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"

#include "argv.h"
#include "arg_script.h"
#include "executable.h"
#include "file_stream.h"
#include "function.h"
#include "variable_map.h"

Arg_script_t::Arg_script_t(const Rwsh_istream_p& input_i,
    const Rwsh_ostream_p& output_i, const Rwsh_ostream_p& error_i) :
  argfunction(0), argfunction_level(0), input(input_i), output(output_i),
  error(error_i) {}

Arg_script_t::Arg_script_t(const std::string& src, unsigned max_soon) :
  argfunction(0), argfunction_level(0),
  input(default_input), output(default_output), error(default_error) {
  std::string::size_type token_start = src.find_first_not_of(" "); 
  while (token_start != std::string::npos) {
    std::string::size_type token_end = src.find_first_of("{} \\", token_start);
    for (; token_end != std::string::npos;
         token_end = src.find_first_of("{} \\", token_end+2))
      if (src[token_end] != '\\') break;
      else if (token_end+1 == src.length()) {token_end = std::string::npos; break; throw Line_continuation_t();}
      else;
    if (token_end == std::string::npos) {
      add_token(src, token_start, std::string::npos, max_soon);
      break;}
    else switch (src[token_end]) {
      case ' ':
        if (src[token_start] == '(') {
          add_literal(src, token_start, max_soon);
          token_start = src.find_first_not_of(" ", token_start+1);}
        else {
          add_token(src, token_start, token_end, max_soon);
          token_start = src.find_first_not_of(" ", token_end);}
        break;
      case '{': {
        std::string::size_type close_brace = find_close_brace(src, token_end);
        if (close_brace == std::string::npos)
          throw Unclosed_brace_t(src.substr(0, token_end+1));
        add_function(src.substr(token_start, token_end-token_start),
                     src.substr(token_end+1, close_brace-token_end-1), 
                     max_soon);
        token_start = src.find_first_not_of(" ", close_brace+1);
        break;}
      case '}': throw Mismatched_brace_t(src.substr(0, token_end+1));
      default: assert(0);}} // error in std::string::find_first_of
  if (!args.size()) args.push_back(Arg_spec_t("", max_soon));
  if (is_argfunction_name(args.front().str()) &&  // argfunction_level handling
      args.front().str() != "rwsh.mapped_argfunction") {
    if (args.size() != 1 || argfunction) 
      throw Arguments_to_argfunction_t(args.front().str());
    if (args.front().str() == "rwsh.unescaped_argfunction")
      argfunction_level = 1;
    else if (args.front().str() == "rwsh.argfunction") argfunction_level = 2;
    else if (args.front().str() == "rwsh.escaped_argfunction")
      argfunction_level = 3;
    else assert(0);}}                            // unhandled argfunction level

void Arg_script_t::add_literal(const std::string& src,
                               std::string::size_type& token_start,
                               unsigned max_soon) {
  std::string::size_type token_end = src.find_first_of("()", token_start+1); 
  for (unsigned nesting = 0;
       token_end != std::string::npos && (nesting || src[token_end] == '(');
       token_end = src.find_first_of("()", token_end+1))
    if (src[token_end] == '(') ++nesting;
    else --nesting;
  if (token_end == std::string::npos)
    throw Unclosed_parenthesis_t(src.substr(0, token_start+1));
  else {
    std::string literal = src.substr(token_start+1, token_end-token_start-1);
    args.push_back(Arg_spec_t(literal, max_soon));}
  token_start = token_end;}

void Arg_script_t::add_token(const std::string& src,
                             std::string::size_type token_start,
                             std::string::size_type token_end,
 unsigned max_soon) {
  if (src[token_start] == '<')
    if (!input.is_default())
      throw Double_redirection_t(input.str(), src.substr(token_start, token_end-token_start));
    else {
      std::string name(src.substr(token_start+1, token_end-token_start-1));
      input = Rwsh_istream_p(new File_istream_t(name), false, false);}
  else if (src[token_start] == '>')
    if (!output.is_default())
      throw Double_redirection_t(output.str(), src.substr(token_start, token_end-token_start));
    else {
      std::string name(src.substr(token_start+1, token_end-token_start-1));
      output = Rwsh_ostream_p(new File_ostream_t(name), false, false);}
  else if (src[token_start] == '(') {
    add_literal(src, token_start, max_soon);
    token_start = src.find_first_not_of(" ", token_start+1);}
  else if (src[token_start] == ')')
    throw Mismatched_parenthesis_t(src.substr(0, token_start+1));
  else args.push_back(Arg_spec_t(src.substr(token_start, token_end-token_start), max_soon));}

void Arg_script_t::add_function(const std::string& style, 
                                const std::string& f_str, unsigned max_soon) {
  if (style.size()) args.push_back(Arg_spec_t(style, f_str, max_soon));
  else
    if (argfunction) throw Multiple_argfunctions_t();
    else argfunction = new Function_t("rwsh.argfunction", f_str, max_soon+1);}

std::string::size_type 
Arg_script_t::find_close_brace(const std::string& focus,
                               std::string::size_type i) {
  unsigned nesting = 1;
  while (nesting && (i = focus.find_first_of("{}", i+1)) != std::string::npos) {
    if (focus[i] == '{') ++nesting;
    else --nesting;}
  return i;}

Arg_script_t::Arg_script_t(const Arg_script_t& src) : 
  args(src.args), argfunction(src.argfunction->copy_pointer()),
  argfunction_level(src.argfunction_level), input(src.input), 
  output(src.output), error(src.error) {}

Arg_script_t& Arg_script_t::operator=(const Arg_script_t& src) {
  args.clear();
  copy(src.args.begin(), src.args.end(), std::back_inserter(args));
  delete argfunction; 
  argfunction = src.argfunction->copy_pointer();
  argfunction_level = src.argfunction_level;
  input = src.input;
  output = src.output;
  error = src.error;}

Arg_script_t::~Arg_script_t(void) {
  delete argfunction;}

// naively create an Argv_t from Arg_script. string constructor for Argv_t.
Argv_t Arg_script_t::argv(void) const {
  Argv_t result;
  if (!argfunction_level) {
    for(std::vector<Arg_spec_t>::const_iterator i=args.begin();
      i != args.end(); ++i) result.push_back(i->str());
    result.set_argfunction(argfunction->copy_pointer());
    result.input = input;
    result.output = output;
    result.error = error;}
  else if (argfunction_level == 1) result.push_back("rwsh.argfunction");
  else if (argfunction_level == 2) result.push_back("rwsh.escaped_argfunction");
  else assert(0); // unhandled argfunction_level
  return result;}

// create a string from Arg_script. inverse of string constructor.
std::string Arg_script_t::str(void) const {
  if (!argfunction_level) {
    std::string result = args[0].str();
    if (args.size() == 1 && result == "()") result.clear();
    for(std::vector<Arg_spec_t>::const_iterator i=args.begin()+1;
        i != args.end(); ++i) result += ' ' + i->str();
    if (!input.is_default()) result += (result.size()?" ":"") + input.str();
    if (!output.is_default()) result += (result.size()?" ":"") + output.str();
    if (!error.is_default()) result += (result.size()?" ":"") + error.str();
    if (argfunction) result += (result.size()?" ":"") + argfunction->str();
    return result;}
  else if (argfunction_level == 1) return "rwsh.argfunction";
  else if (argfunction_level == 2) return "rwsh.escaped_argfunction";
  else {assert(0); return "";}} // unhandled argfunction_level

// produce a destination Argv from the source Argv according to this script
Argv_t Arg_script_t::interpret(const Argv_t& src) const {
  Argv_t result;
  if (!input.is_default()) result.input = input;
  else result.input = src.input.child_stream();
  if (!output.is_default()) result.output = output;
  else result.output = src.output.child_stream();
  if (!error.is_default()) result.error = error;
  else result.error = src.error.child_stream();
  if (!argfunction_level) {
    for (std::vector<Arg_spec_t>::const_iterator i = args.begin();
      i != args.end(); ++i) 
      i->interpret(src, std::back_inserter(result));
    if (!result.size()) result.push_back("");
    if (argfunction) result.set_argfunction(argfunction->apply(src, 0));}
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
void Arg_script_t::apply(const Argv_t& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_script_t> > res) const {
  if (this->argfunction_level) {
    Arg_script_t result(*this);
    --result.argfunction_level;
    *res++ = result;}
  else {
    Arg_script_t result(input, output, error);
    for (std::vector<Arg_spec_t>::const_iterator i = args.begin();
         i != args.end(); ++i) 
      i->apply(src, nesting, std::back_inserter(result.args));
    if (argfunction) result.argfunction = argfunction->apply(src, nesting+1);
    *res++ = result;}}

void Arg_script_t::promote_soons(unsigned nesting) {
  for (std::vector<Arg_spec_t>::iterator i = args.begin(); i != args.end();
       ++i) i->promote_soons(nesting);
  if (argfunction) {
    Function_t* temp = argfunction->promote_soons(nesting);
    delete argfunction;
    argfunction = temp;}}

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
  return !focus.compare(0, 1, "/");}

// test whether an executable name is possible for a builtin, does not test 
// whether this builtin exists
bool is_builtin_name(const std::string& focus) {
  return !focus.compare(0, 1, "%");}

// test whether this is an appropriate name for an internal function
bool is_internal_function_name(const std::string& focus) {
  return !focus.compare(0, 5, "rwsh.");}

// test whether an executable name is possible for a function
bool is_function_name(const std::string& focus) {
  return !is_binary_name(focus) && !is_builtin_name(focus) && 
    !is_internal_function_name(focus);}

