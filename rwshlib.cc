// Copyright (C) 2005-2008 Samuel Newbold

#include <limits>
#include <string>
#include <sys/errno.h>

#include "rwshlib.h"

double my_strtod(const std::string& src) {
  const char* focus = src.c_str();
  char* endptr;
  errno = 0;
  double ret = strtold(focus, &endptr);
  if (!*focus || *endptr) throw E_nan();
  if (errno == ERANGE) {errno = 0; throw E_range();}
  else if (ret == std::numeric_limits<double>::infinity() ||
           ret == -std::numeric_limits<double>::infinity()) throw E_range();
  else if (errno) {errno = 0; throw E_generic();}
  else return ret;}

float my_strtof(const std::string& src) {
  const char* focus = src.c_str();
  char* endptr;
  errno = 0;
  float ret = strtof(focus, &endptr);
  if (!*focus || *endptr) throw E_nan();
  if (errno == ERANGE) {errno = 0; throw E_range();}
  else if (ret == std::numeric_limits<float>::infinity() ||
           ret == -std::numeric_limits<float>::infinity()) throw E_range();
  else if (errno) {errno = 0; throw E_generic();}
  else return ret;}

int my_strtoi(const std::string& src) {
  const char* focus = src.c_str();
  char* endptr;
  errno = 0;
  long ret = strtol(focus, &endptr, 10);
  if (!*focus || *endptr) throw E_nan();
  if (errno == ERANGE) throw E_range();
  else if (ret < INT_MIN) throw E_range();
  else if (ret > INT_MAX) throw E_range();
  else if (errno) throw E_generic();
  else return ret;}

int my_strtoi(const std::string& src, int min, int max) {
  const char* focus = src.c_str();
  char* endptr;
  errno = 0;
  long ret = strtol(focus, &endptr, 10);
  if (!*focus || *endptr) throw E_nan();
  if (errno == ERANGE) throw E_range();
  else if (ret < min) throw E_range();
  else if (ret > max) throw E_range();
  else if (errno) throw E_generic();
  else return ret;}

char my_strtoc(const std::string& src) {
  const char* focus = src.c_str();
  char* endptr;
  errno = 0;
  long ret = strtol(focus, &endptr, 10);
  if (!*focus || *endptr) throw E_nan();
  if (errno == ERANGE) throw E_range();
  else if (ret < CHAR_MIN) throw E_range();
  else if (ret > CHAR_MAX) throw E_range();
  else if (errno) throw E_generic();
  else return ret;}

