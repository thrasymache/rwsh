// The definition of the Argm class, which contains the arguments that may
// be passed to an executable.
//
// Copyright (C) 2005-2019 Samuel Newbold

#include <cstring>
#include <cstdlib>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "rwshlib.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "call_stack.h"
#include "executable.h"
#include "prototype.h"

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

void Argm::set_argfunction(Command_block* val) {argfunction_v = val;};

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
      catch (E_nan ex) {throw Exception(Argm::Undefined_variable, key);}}
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
    default: return parent_map()->exists(key, true);}}

void Argm::global(const std::string& key, const std::string& value) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      throw Exception(Argm::Illegal_variable_name, key);
    default: parent_map()->global(key, value);}}

void Argm::local(const std::string& key, const std::string& value) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      throw Exception(Argm::Illegal_variable_name, key);
    default: parent_map()->local(key, value);}}

void Argm::local_declare(const std::string& key, Error_list& exceptions) const{
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      exceptions.add_error(Exception(Argm::Illegal_variable_name, key));
      break;
    default: parent_map()->local_declare(key);}}

void Argm::locals_listed(void) const {
  parent_map()->locals_listed = true;}

void Argm::unset_var(const std::string& key) const {
  switch (key[0]) {
    case '#': case '*': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9': case '0':
      throw Exception(Argm::Illegal_variable_name, key);
    default: parent_map()->unset(key);}}

// algorithm that is the meat of Old_argv constructor
template<class In>char** copy_to_cstr(In first, In last, char** res) {
  while (first != last) {
    *res = new char[(*first).length()+1];
    strcpy(*res++, (*first++).c_str());}
  *res = 0;
  return res;}

Exception::Exception(Exception_t exception_i) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);}

Exception::Exception(Exception_t exception_i, const std::string& value) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(value);}

Exception::Exception(Exception_t exception_i, const std::string& value,
                         int errno_v) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(value);
  std::ostringstream errno_str;
  errno_str <<errno_v;
  push_back(errno_str.str());}

Exception::Exception(Exception_t exception_i, const std::string& x,
                         const std::string& y) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(x);
  push_back(y);}

Exception::Exception(Exception_t exception_i, const std::string& w,
                         const std::string& x, const std::string& y,
                         const std::string& z) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(w);
  push_back(x);
  push_back(y);
  push_back(z);}

Exception::Exception(Exception_t exception_i, int x) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  std::ostringstream x_str;
  x_str <<x;
  push_back(x_str.str());}

Exception::Exception(Exception_t exception_i, int x, int y) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  std::ostringstream x_str, y_str;
  x_str <<x;
  push_back(x_str.str());
  y_str <<y;
  push_back(y_str.str());}

Exception::Exception(Exception_t exception_i, int x, int y, int z) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  std::ostringstream x_str, y_str, z_str;
  x_str <<x;
  push_back(x_str.str());
  y_str <<y;
  push_back(y_str.str());
  z_str <<z;
  push_back(z_str.str());}

Exception::Exception(Exception_t exception_i, const Argm& src) :
    Argm(Variable_map::global_map, default_input,default_output,default_error),
    exception(exception_i) {
  push_back(exception_names[exception]);
  std::copy(src.begin(), src.end(), std::back_inserter(*this));}

void Error_list::reset(void) {
  clear();
  global_stack.reset();}

void Error_list::add_error(const Argm& error){
  push_back(error);
  global_stack.add_error();}

void Error_list::replace_error(const Argm& error){
  push_back(error);
  global_stack.replace_error();}

Old_argv::Old_argv(const Argm::Argv& src) : argc_v(src.size()) {
  focus = new char*[src.size()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv::~Old_argv(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}

