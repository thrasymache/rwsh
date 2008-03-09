// Copyright (C) 2007 Samuel Newbold

#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "rwsh_stream.h"

#include "plumber.h"
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
  int filenos[2];
  if (pipe(filenos)) std::cerr <<"failed pipe with errno " <<errno <<std::endl;
  plumber.proxy_output(filenos[0], this);
  plumber.close_on_wait(filenos[1]);
  return filenos[1];}

std::string Substitution_stream_t::str(void) const {
  return "&{}";}
