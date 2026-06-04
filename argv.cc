// The definition of the Exception class, which contains a sequence arguments
// that will be used to construct an Argm, and Old_argv, which is what system
// calls expect when running external programs. 
//
// Copyright (C) 2005-2026 Samuel Newbold

#include <cstring>
#include <string>
#include <sstream>
#include <vector>

#include "argv.h"

Exception::Exception(E::Exception_t exception_i) :
    exception(exception_i) {
  push_back(exception_names[exception]);}

Exception::Exception(E::Exception_t exception_i, const std::string& value) :
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(value);}

Exception::Exception(E::Exception_t exception_i, const std::string& value,
                         int errno_v) :
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(value);
  std::ostringstream errno_str;
  errno_str <<errno_v;
  push_back(errno_str.str());}

Exception::Exception(E::Exception_t exception_i, const std::string& x,
                         const std::string& y) :
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(x);
  push_back(y);}

Exception::Exception(E::Exception_t exception_i, const std::string& w,
                         const std::string& x, const std::string& y,
                         const std::string& z) :
    exception(exception_i) {
  push_back(exception_names[exception]);
  push_back(w);
  push_back(x);
  push_back(y);
  push_back(z);}

Exception::Exception(E::Exception_t exception_i, int x) :
    exception(exception_i) {
  push_back(exception_names[exception]);
  std::ostringstream x_str;
  x_str <<x;
  push_back(x_str.str());}

Exception::Exception(E::Exception_t exception_i, int x, int y) :
    exception(exception_i) {
  push_back(exception_names[exception]);
  std::ostringstream x_str, y_str;
  x_str <<x;
  push_back(x_str.str());
  y_str <<y;
  push_back(y_str.str());}

Exception::Exception(E::Exception_t exception_i, int x, int y, int z) :
    exception(exception_i) {
  push_back(exception_names[exception]);
  std::ostringstream x_str, y_str, z_str;
  x_str <<x;
  push_back(x_str.str());
  y_str <<y;
  push_back(y_str.str());
  z_str <<z;
  push_back(z_str.str());}

// algorithm that is the meat of Old_argv constructor
template<class In>char** copy_to_cstr(In first, In last, char** res) {
  while (first != last) {
    *res = new char[(*first).length()+1];
    strcpy(*res++, (*first++).c_str());}
  *res = 0;
  return res;}

Old_argv::Old_argv(const Argv& src) : argc_v(src.size()) {
  focus = new char*[src.size()+1];
  copy_to_cstr(src.begin(), src.end(), focus);}

Old_argv::~Old_argv(void) {
  if (!focus) return;
  for (char** i=focus; *i; ++i) delete[] *i;
  delete[] focus;}



