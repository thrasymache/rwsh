// The definition of the Arg_script class which contains a vector of
// Arg_spec objects and a Function*, and which represents the mapping
// between the Argm that was passed to the function and the Argm that will
// be used to run a given executable.
//
// Copyright (C) 2006-2023 Samuel Newbold

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <sys/time.h>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "call_stack.h"
#include "executable.h"
#include "file_stream.h"
#include "prototype.h"

#include "function.h"

namespace {
const char* all_separators = " \t\n\\{};";
} // close unnamed namespace

std::string::size_type Arg_script::add_quote(const std::string& src,
        std::string::size_type point, unsigned max_soon, Error_list& errors) {
  std::string literal;
  const char* separators = "\\()";
  auto split = src.find_first_of(separators, point+1);
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
  if (split == std::string::npos) {
    errors.add_error(Exception(Argm::Unclosed_parenthesis,
                               src.substr(0, point+1)));
    return std::string::npos;}
  else {
    literal += src.substr(point+1, split-point-1);
    point = src.find_first_not_of(WSPACE, split+1);
    if (literal[0] != '\\') add_token(literal, max_soon, errors);
    else add_token("\\" + literal, max_soon, errors);
    return point;}}

std::string::size_type Arg_script::parse_token(const std::string& src,
    std::string::size_type token_start, unsigned max_soon, Error_list& errors) {
  if (src[token_start] == '(')
    return add_quote(src, token_start, max_soon, errors);
  else if (src[token_start] == ')')
    errors.add_error(Exception(Argm::Mismatched_parenthesis,
                      src.substr(0, token_start+1)));
  auto split = src.find_first_of(all_separators, token_start),
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
    add_token(token, max_soon, errors);
    return point;}
  else if (src[split] == '{') {
    token_start = add_braced_token(src, token_start, split, max_soon, errors);
    return src.find_first_not_of(WSPACE, token_start);}
  else {
    point = src.find_first_not_of(WSPACE, split);
    add_token(token, max_soon, errors);
    return point;}}

Arg_script::Arg_script(const Rwsh_istream_p& input_i,
    const Rwsh_ostream_p& output_i, const Rwsh_ostream_p& error_i,
    const std::string& indent_i, char terminator_i) :
  argfunction(0), argfunction_level(0), input(input_i), output(output_i),
  error(error_i), indent(indent_i), terminator(terminator_i) {}

Arg_script::Arg_script(const std::string& src, std::string::size_type& point,
                       unsigned max_soon, Error_list& errors) :
  argfunction(0), argfunction_level(0), input(default_input),
  output(default_output), error(default_error), terminator('!') {
  auto tpoint = src.find_first_not_of(WSPACE, point);
  indent = src.substr(point, tpoint-point);
  point = constructor(src, tpoint, max_soon, errors);}

std::string::size_type Arg_script::constructor(const std::string& src,
          std::string::size_type point, unsigned max_soon, Error_list& errors) {
  for (; point < src.length();
       point = parse_token(src, point, max_soon, errors)) switch (src[point]) {
    default: break;
    case ';': case '\n': case '}': goto after_loop;} // statement terminators
  after_loop:
  if (!args.size()) args.push_back(Arg_spec("", max_soon, errors));
  if (is_argfunction_name(args.front().str()) &&  // argfunction_level handling
      args.front().str() != ".mapped_argfunction")
    if (args.size() != 1 || argfunction)
      errors.add_error(Exception(Argm::Arguments_for_argfunction, str()));
    else if (args.front().str() == ".unescaped_argfunction")
      argfunction_level = 1;
    else if (args.front().str() == ".argfunction") argfunction_level = 2;
    else if (args.front().str() == ".escaped_argfunction")
      argfunction_level = 3;
    else std::abort();                          // unhandled argfunction level
  else;
  if (point >= src.length()) {
    terminator = *"";
    point = std::string::npos;}
  else if (src[point] == '}') terminator = *"";
  else terminator = src[point];
  return point;}

void Arg_script::add_token(const std::string& src, unsigned max_soon,
                           Error_list& errors) {
  switch (src[0]) {
    case '<':
      if (!input.is_default())
        errors.add_error(Exception(Argm::Double_redirection, input.str(), src));
      else input = Rwsh_istream_p(new File_istream(src.substr(1)),
                                  false, false);
      break;
    case '>':
      if (!output.is_default())
        errors.add_error(Exception(Argm::Double_redirection, output.str(),src));
      else output = Rwsh_ostream_p(new File_ostream(src.substr(1)),
                                   false, false);
      break;
    default:
      args.push_back(Arg_spec(src, max_soon, errors));}}

std::string::size_type Arg_script::add_braced_token(const std::string& src,
        std::string::size_type style_start, std::string::size_type point,
        unsigned max_soon, Error_list& errors) {
  if (style_start != point)
    args.push_back(Arg_spec(src, style_start, point, max_soon, errors));
  else if (argfunction) {
    errors.add_error(Exception(Argm::Multiple_argfunctions));
    delete argfunction;
    argfunction = new Command_block(src, point, max_soon+1, errors);}
  else argfunction = new Command_block(src, point, max_soon+1, errors);
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
  terminator = src.terminator;
  return *this;}

Arg_script::~Arg_script(void) {
  delete argfunction;}

// naively create an Argm from Arg_script. string constructor for Argm.
Argm Arg_script::argm(void) const {
  Argm result(Variable_map::global_map, input, output, error);
  if (!argfunction_level) {
    for(auto j: args) result.push_back(j.str());
    result.set_argfunction(argfunction->copy_pointer());}
  else if (argfunction_level == 1) result.push_back(".argfunction");
  else if (argfunction_level == 2) result.push_back(".escaped_argfunction");
  else std::abort(); // unhandled argfunction_level
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
  else if (argfunction_level == 1) return ".argfunction";
  else if (argfunction_level == 2) return ".escaped_argfunction";
  else if (argfunction_level == 3) return ".super_escaped_argfunction";
  else {std::abort(); return "";}} // unhandled argfunction_level

// produce a destination Argm from the source Argm according to this script
Argm Arg_script::interpret(const Argm& src, Error_list& exceptions) const {
  Argm result(src.parent_map(),
              !input.is_default()?  input:  src.input.child_stream(),
              !output.is_default()? output: src.output.child_stream(),
              !error.is_default()?  error:  src.error.child_stream());
  if (!argfunction_level) {
    for (auto j: args) j.interpret(src, std::back_inserter(result), exceptions);
    if (!result.argc()) result.push_back("");
    if (argfunction)
      result.set_argfunction(argfunction->apply(src, 0, exceptions));}
  else if (is_argfunction()) {
    result.push_back(".mapped_argfunction");
    copy(src.begin()+1, src.end(), std::back_inserter(result));
    result.set_argfunction(src.argfunction()->copy_pointer());}
  else if (argfunction_level == 2)
    result.push_back(".unescaped_argfunction");
  else if (argfunction_level == 3) result.push_back(".argfunction");
  else std::abort(); // unhandled argfunction_level
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
    for (auto j: args)
      j.apply(src, nesting, std::back_inserter(result.args), exceptions);
    if (argfunction)
      result.argfunction = argfunction->apply(src, nesting+1, exceptions);
    *res++ = result;}}

void Arg_script::promote_soons(unsigned nesting) {
  for (auto j = args.begin(); j != args.end(); ++j) j->promote_soons(nesting);
  argfunction->promote_soons(nesting);}

// test whether an executable name corresponds to one of those used for
// argument functions.
bool is_argfunction_name(const std::string& focus) {
  return focus == ".mapped_argfunction" ||
         focus == ".unescaped_argfunction" ||
         focus == ".argfunction" ||
         focus == ".escaped_argfunction";}
