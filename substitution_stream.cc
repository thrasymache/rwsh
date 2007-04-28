// Copyright (C) 2007 Samuel Newbold

#include <sstream>
#include <string>
#include <vector>

#include "rwsh_stream.h"
#include "substitution_stream.h"

Rwsh_stream_t& Substitution_stream_t::operator<<(const std::string& r) {
  buffer <<r; 
  return *this;}

Rwsh_stream_t& Substitution_stream_t::operator<<(int r) {
  buffer <<r; 
  return *this;}

