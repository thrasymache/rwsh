// Utility function to find the next close brace at the same nesting level. 
//
// Copyright (C) 2006, 2007 Samuel Newbold

#include <sys/errno.h>
#include <string>

#include "util.h"

std::string::size_type find_close_brace(const std::string& focus,
                                        std::string::size_type i) {
  unsigned nesting = 1;
  while (nesting && (i = focus.find_first_of("{}", i+1)) != std::string::npos) {
    if (focus[i] == '{') ++nesting;
    else --nesting;}
  return i;}

int my_strtoi(const std::string& src) {
  const char* focus = src.c_str();
  char* endptr;
  errno = 0;
  long ret = strtol(focus, &endptr, 10);
  if (!*focus || *endptr) throw E_nan_t();
  if (errno == ERANGE) throw E_range_t();
  else if (ret < INT_MIN) throw E_range_t();
  else if (ret > INT_MAX) throw E_range_t();
  else if (errno) throw E_generic_t();
  else return ret;}

