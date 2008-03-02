// Copyright (C) 2007 Samuel Newbold

#include <assert.h>
#include <sstream>
#include <string>
#include <vector>

#include "rwsh_stream.h"
#include "substitution_stream.h"

Rwsh_ostream_t* Substitution_stream_t::copy_pointer(void) {
  assert(0);} // substitution streams are not used in this way

Rwsh_ostream_t& Substitution_stream_t::operator<<(const std::string& r) {
  buffer <<r; 
  return *this;}

Rwsh_ostream_t& Substitution_stream_t::operator<<(int r) {
  buffer <<r; 
  return *this;}

bool Substitution_stream_t::fail(void) {return false;}

int Substitution_stream_t::fileno(void) {
  assert(0);}

std::string Substitution_stream_t::str(void) const {
  return "&{}";}
