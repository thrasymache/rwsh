// The definition of the Arg_script class which contains a vector of
// Arg_spec objects and a Function*, and which represents the mapping
// between the Argm that was passed to the function and the Argm that will
// be used to run a given executable.
//
// Copyright (C) 2006-2017 Samuel Newbold

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "executable.h"
#include "file_stream.h"
#include "prototype.h"

#include "function.h"

namespace {
const char* all_separators = " \t\n\\{};";
} // close unnamed namespace

std::string::size_type Arg_script::add_quote(const std::string& src,
                                             std::string::size_type point,
                                             unsigned max_soon) {
  std::string literal;
  const char* separators = "\\()";
  std::string::size_type split = src.find_first_of(separators, point+1);
  for (unsigned nesting = 0;
       split != std::string::npos && (nesting || src[split] != ')');
       split = src.find_first_of(separators, split+1)) switch(src[split]) {
    case '(': ++nesting; break;
    case ')': --nesting; break;
    case '\\':
      literal += src.substr(point+1, split-point-1);
      point = split;
      split += 1;
      break;
    default: std::abort();}
  if (split == std::string::npos)
    throw Unclosed_parenthesis(src.substr(0, point+1));
  else {
    literal += src.substr(point+1, split-point-1);
    point = src.find_first_not_of(WSPACE, split+1);
    if (literal[0] != '\\') add_token(literal, max_soon);
    else add_token("\\" + literal, max_soon);
    return point;}}

std::string::size_type Arg_script::parse_token(const std::string& src,
                                            std::string::size_type token_start,
                                            unsigned max_soon) {
  if (src[token_start] == '(')
    return add_quote(src, token_start, max_soon);
  else if (src[token_start] == ')')
    throw Exception(Argm::Mismatched_parenthesis,
                      src.substr(0, token_start+1));
  std::string::size_type split = src.find_first_of(all_separators, token_start),
                         point = token_start;
  std::string token;
  for (; split != std::string::npos;       // loop until an unescaped separator
       split = src.find_first_of(all_separators, split+2))
    if (src[split] != '\\') break;
    else if (split+1 == src.length()) {
      split = std::string::npos;
      //throw Line_continuation();
      break;}
    else if (split != token_start) {
      token += src.substr(point, split-point);
      point = split+1;}
  token += src.substr(point, split-point);
  if (split == std::string::npos) {
    point = split;
    add_token(token, max_soon);
    return point;}
  else if (src[split] == '{') {
    token_start = add_function(src, token_start, split, max_soon);
    return src.find_first_not_of(WSPACE, token_start);}
  else {
    point = src.find_first_not_of(WSPACE, split);
    add_token(token, max_soon);
    return point;}}

Arg_script::Arg_script(const Rwsh_istream_p& input_i,
    const Rwsh_ostream_p& output_i, const Rwsh_ostream_p& error_i,
    const std::string& indent_i, char terminator_i) :
  argfunction(0), argfunction_level(0), input(input_i), output(output_i),
  error(error_i), indent(indent_i), terminator(terminator_i) {}

Arg_script::Arg_script(const std::string& src, unsigned max_soon) :
  argfunction(0), argfunction_level(0), input(default_input),
  output(default_output), error(default_error), terminator('!') {
  std::string::size_type point = src.find_first_not_of(WSPACE, 0);
  indent = src.substr(0, point);
  point = constructor(src, point, max_soon);
  if (point < src.length())
    if (src[point] == '}' || src[point] == ';')
      throw Exception(Argm::Mismatched_brace, src.substr(0, point+1));
    else std::abort();}

Arg_script::Arg_script(const std::string& src, std::string::size_type& point,
                       unsigned max_soon) :
  argfunction(0), argfunction_level(0), input(default_input),
  output(default_output), error(default_error), terminator('!') {
  std::string::size_type tpoint = src.find_first_not_of(WSPACE, point);
  indent = src.substr(point, tpoint-point);
  point = constructor(src, tpoint, max_soon);}

std::string::size_type Arg_script::constructor(const std::string& src,
                              std::string::size_type point, unsigned max_soon) {
  for (; point < src.length();
       point = parse_token(src, point, max_soon)) switch (src[point]) {
    default: break;
    case ';': case '\n': case '}': goto after_loop;} // statement terminators
  after_loop:
  if (!args.size()) args.push_back(Arg_spec("", max_soon));
  if (is_argfunction_name(args.front().str()) &&  // argfunction_level handling
      args.front().str() != "rwsh.mapped_argfunction")
    if (args.size() != 1 || argfunction)
      throw Exception(Argm::Arguments_for_argfunction, args.front().str());
    else if (args.front().str() == "rwsh.unescaped_argfunction")
      argfunction_level = 1;
    else if (args.front().str() == "rwsh.argfunction") argfunction_level = 2;
    else if (args.front().str() == "rwsh.escaped_argfunction")
      argfunction_level = 3;
    else std::abort();                           // unhandled argfunction level
  if (src[point] == '}') terminator = *"";
  else terminator = src[point];
  return point;}

void Arg_script::add_token(const std::string& src, unsigned max_soon) {
  switch (src[0]) {
    case '<':
      if (!input.is_default())
        throw Exception(Argm::Double_redirection, input.str(), src);
      else input = Rwsh_istream_p(new File_istream(src.substr(1)),
                                  false, false);
      break;
    case '>':
      if (!output.is_default())
        throw Exception(Argm::Double_redirection, output.str(), src);
      else output = Rwsh_ostream_p(new File_ostream(src.substr(1)),
                                   false, false);
      break;
    default:
      args.push_back(Arg_spec(src, max_soon));}}

std::string::size_type Arg_script::add_function(const std::string& src,
                                             std::string::size_type style_start,
                                             std::string::size_type point,
                                             unsigned max_soon) {
  if (style_start != point)
    args.push_back(Arg_spec(src, style_start, point, max_soon));
  else
    if (argfunction) throw Exception(Argm::Multiple_argfunctions);
    else argfunction = new Command_block(src, point, max_soon+1);
  return point;}

Arg_script::Arg_script(const Arg_script& src) :
  args(src.args), argfunction(src.argfunction->copy_pointer()),
  argfunction_level(src.argfunction_level),
  input(src.input), output(src.output), error(src.error),
  indent(src.indent), terminator(src.terminator) {}

Arg_script& Arg_script::operator=(const Arg_script& src) {
  args.clear();
  copy(src.args.begin(), src.args.end(), std::back_inserter(args));
  delete argfunction;
  argfunction = src.argfunction->copy_pointer();
  argfunction_level = src.argfunction_level;
  input = src.input;
  output = src.output;
  error = src.error;
  indent = src.indent;
  terminator = src.terminator;}

Arg_script::~Arg_script(void) {
  delete argfunction;}

// naively create an Argm from Arg_script. string constructor for Argm.
Argm Arg_script::argm(void) const {
  Argm result(Variable_map::global_map, input, output, error);
  if (!argfunction_level) {
    for(auto i=args.begin(); i != args.end(); ++i) result.push_back(i->str());
    result.set_argfunction(argfunction->copy_pointer());}
  else if (argfunction_level == 1) result.push_back("rwsh.argfunction");
  else if (argfunction_level == 2) result.push_back("rwsh.escaped_argfunction");
  else abort(); // unhandled argfunction_level
  return result;}

// create a string from Arg_script. inverse of string constructor.
std::string Arg_script::str(void) const {
  if (!argfunction_level) {
    std::string result = indent + args[0].str();
    if (args.size() == 1 && result == "()") result.clear();
    for(auto i=args.begin()+1; i != args.end(); ++i) result += ' ' + i->str();
    if (!input.is_default()) result += (result.size()?" ":"") + input.str();
    if (!output.is_default()) result += (result.size()?" ":"") + output.str();
    if (!error.is_default()) result += (result.size()?" ":"") + error.str();
    if (argfunction) result += (result.size()?" ":"") + argfunction->str();
    if (terminator) return result + terminator;
    else return result;}
  else if (argfunction_level == 1) return "rwsh.argfunction";
  else if (argfunction_level == 2) return "rwsh.escaped_argfunction";
  else {abort(); return "";}} // unhandled argfunction_level

Argm Arg_script::base_interpret(const Argm& src) const {
  Error_list exceptions;
  Argm ret = interpret(src, exceptions);
  if (exceptions.size()) Base_executable::exception_handler(exceptions);
  return ret;}

// produce a destination Argm from the source Argm according to this script
Argm Arg_script::interpret(const Argm& src, Error_list& exceptions) const {
  Argm result(src.parent_map(),
              !input.is_default()?  input:  src.input.child_stream(),
              !output.is_default()? output: src.output.child_stream(),
              !error.is_default()?  error:  src.error.child_stream());
  if (!argfunction_level) {
    for (auto i = args.begin(); i != args.end(); ++i)
      i->interpret(src, std::back_inserter(result), exceptions);
    if (!result.argc()) result.push_back("");
    if (argfunction)
      result.set_argfunction(argfunction->apply(src, 0, exceptions));}
  else if (is_argfunction()) {
    result.push_back("rwsh.mapped_argfunction");
    copy(src.begin()+1, src.end(), std::back_inserter(result));
    result.set_argfunction(src.argfunction()->copy_pointer());}
  else if (argfunction_level == 2)
    result.push_back("rwsh.unescaped_argfunction");
  else if (argfunction_level == 3) result.push_back("rwsh.argfunction");
  else abort(); // unhandled argfunction_level
  return result;}

// produce a new Arg_script by unescaping argument functions and replacing
// unescaped_argfunction with argm.argfunction
void Arg_script::apply(const Argm& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_script> > res,
             Error_list& exceptions) const {
  if (this->argfunction_level) {
    Arg_script result(*this);
    --result.argfunction_level;
    *res++ = result;}
  else {
    Arg_script result(input, output, error, indent, terminator);
    for (auto i = args.begin(); i != args.end(); ++i)
      i->apply(src, nesting, std::back_inserter(result.args), exceptions);
    if (argfunction)
      result.argfunction = argfunction->apply(src, nesting+1, exceptions);
    *res++ = result;}}

void Arg_script::promote_soons(unsigned nesting) {
  for (auto i = args.begin(); i != args.end(); ++i) i->promote_soons(nesting);
  argfunction->promote_soons(nesting);}

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

// test whether this is an appropriate name for an internal function
bool is_internal_function_name(const std::string& focus) {
  return !focus.compare(0, 5, "rwsh.");}

// test whether an executable name is possible for a function
bool is_function_name(const std::string& focus) {
  return !is_binary_name(focus) && !is_internal_function_name(focus);}

