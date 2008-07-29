// Copyright (C) 2007 Samuel Newbold

#include <assert.h>
#include <errno.h>
#include <iostream>
#include <iomanip>
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

Rwsh_ostream_t& Substitution_stream_t::operator<<(struct timeval r) {
  buffer <<r.tv_sec <<"." <<std::setw(6) <<std::setfill('0') <<r.tv_usec;
  return *this;}

bool Substitution_stream_t::fail(void) {return false;}

int Substitution_stream_t::fd(void) {
  int fds[2];
  if (pipe(fds)) std::cerr <<"failed pipe with errno " <<errno <<std::endl;
  plumber.proxy_output(fds[0], this);
  plumber.close_on_fork(fds[0]);
  plumber.close_on_wait(fds[1]);
  return fds[1];}

std::string Substitution_stream_t::str(void) const {
  return "&{}";}
