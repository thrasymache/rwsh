// The definition of the Argm class, which contains the arguments that may
// be passed to an executable.
//
// Copyright (C) 2005-2023 Samuel Newbold

#include <cstdlib>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <vector>

#include "argv.h"
#include "rwsh_stream.h"
#include "call_stack.h"
#include "prototype.h"
#include "rwshlib.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "executable.h"
#include "function.h"

Argm::Argm(Variable_map* parent_map_i, Rwsh_istream_p input_i,
           Rwsh_ostream_p output_i, Rwsh_ostream_p error_i) :
  argfunction_v(0), input(input_i), output(output_i), error(error_i),
  parent_map_v(parent_map_i) {}

Argm::Argm(const Argm& src) : argv_v(src.argv_v),
  argfunction_v(src.argfunction()->copy_pointer()),
  input(src.input), output(src.output), error(src.error),
  parent_map_v(src.parent_map()) {}

// constructor of Argm from a subrange with streams
Argm::Argm(const Argv& args,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i,
       Rwsh_ostream_p error_i) :
  argv_v(args.begin(), args.end()),
  argfunction_v(argfunction_i->copy_pointer()),
  input(input_i), output(output_i), error(error_i),
  parent_map_v(parent_map_i) {}

// constructor of Argm from an exception with default streams
Argm::Argm(const Exception& args) :
  argv_v(args.begin(), args.end()),
  argfunction_v(0),
  input(default_input), output(default_output), error(default_error),
  parent_map_v(Variable_map::global_map) {}

Argm::Argm(E::Exception_t exception_i, const Argm& src) :
    Argm(Variable_map::global_map, default_input,default_output,default_error) {
  push_back(exception_names[exception_i]);
  std::copy(src.begin(), src.end(), std::back_inserter(*this));}

// constructor of Argm from a subrange with default streams
Argm::Argm(const Argv& args,
       Command_block* argfunction_i, Variable_map* parent_map_i) :
  argv_v(args.begin(), args.end()),
  argfunction_v(argfunction_i->copy_pointer()),
  input(default_input), output(default_output), error(default_error),
  parent_map_v(parent_map_i) {}

// constructor of Argm from an initial argument and a  pair of iterators
Argm::Argm(const std::string& first_string,
       const Argv& subsequent_args,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i,
       Rwsh_ostream_p error_i) :
  argv_v(subsequent_args.begin(), subsequent_args.end()),
  argfunction_v(argfunction_i->copy_pointer()),
  input(input_i), output(output_i), error(error_i),
  parent_map_v(parent_map_i) {
    argv_v.insert(argv_v.begin(), first_string);}

Argm& Argm::operator=(const Argm& src) {
  argv_v.clear();
  std::copy(src.begin(), src.end(), std::back_inserter(argv_v));
  delete argfunction_v;
  argfunction_v = src.argfunction()->copy_pointer();
  parent_map_v = src.parent_map();
  input = src.input;
  output = src.output;
  error = src.error;
  return *this;}

Argm::~Argm(void) {
  delete argfunction_v;}

// convert to a string. inverse of constructor.
std::string Argm::str(void) const {
  std::string result;
  for (auto i: subrange(0, 1)) result += i + ' ';
  result += back();
  if (!input.is_default()) result += " " + input.str();
  if (!output.is_default()) result += " " + output.str();
  if (!error.is_default()) result += " " + error.str();
  if (argfunction()) result += " " + argfunction()->str();
  return result;}

Variable_map* Argm::nonempty_parent_map(void) const {
    return parent_map_v->nonempty_parent();};

void Argm::set_argfunction(Command_block* val) {argfunction_v = val;};

void Argm::export_env(std::vector<char*>& env) const {
    parent_map()->export_env(env);}

// returns variables that are defined in the current argument map other than $*
// (e.g. positional parameters and $#)
std::string Argm::get_var(const std::string& key) const {
  switch (key[0]) {
    case '#': {
      std::ostringstream str;
      str <<argv_v.size();
      return str.str();}
    case '1': case '2': case '3': case '4': case '5': case '6': case '7':
              case '8': case '9': case '0': {
      try {
        int n = my_strtoi(key.c_str());
        if ((int)argv_v.size() > n) return argv_v[n];
        else return std::string();}
      catch (E_nan ex) {throw Exception(E::Undefined_variable, key);}}
    default: return parent_map()->get(key);}}

void Argm::set_var(const std::string& key, const std::string& value) const {
  parent_map()->set(key, value);}

bool Argm::var_exists(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': return true;
    case '1': case '2': case '3': case '4': case '5': case '6':
              case '7': case '8': case '9': case '0': {
      int n = std::atoi(key.c_str());
      return (int)argv_v.size() > n;}
    default: return parent_map()->exists_with_check(key);}}

void Argm::global(const std::string& key, const std::string& value) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      throw Exception(E::Illegal_variable_name, key);
    default: parent_map()->global(key, value);}}

void Argm::local(const std::string& key, const std::string& value) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      throw Exception(E::Illegal_variable_name, key);
    default: parent_map()->local(key, value);}}

void Argm::local_declare(const std::string& key, Error_list& exceptions) const{
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      exceptions.add_error(Exception(E::Illegal_variable_name, key));
      break;
    default: parent_map()->local_declare(key);}}

void Argm::unset_var(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      throw Exception(E::Illegal_variable_name, key);
    default: parent_map()->unset(key);}}

void Error_list::reset(void) {
  clear();
  global_stack.reset();}

void Error_list::add_error(const Exception& error){
  push_back(Argm(error));
  global_stack.add_error();}

void Error_list::add_error(const Argm& error){
  push_back(error);
  global_stack.add_error();}

void Error_list::replace_error(const Argm& error){
  push_back(error);
  global_stack.replace_error();}
