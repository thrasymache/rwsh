// Copyright (C) 2007 Samuel Newbold

#include <iostream>
#include <string>

#include "rwsh_stream.h"

Rwsh_stream_t& Default_stream_t::operator<<(const std::string& r) {
  std::cout <<r; 
  return *this;}

Rwsh_stream_t& Default_stream_t::operator<<(int r) {
  std::cout <<r; 
  return *this;}

void Default_stream_t::flush(void) {std::cout.flush();}

