// Copyright (C) 2005-2008 Samuel Newbold

#include <string>
#include <sys/errno.h>

#include "rwshlib.h"

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

