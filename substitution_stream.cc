// Copyright (C) 2007-2015 Samuel Newbold

#include <cstdlib>
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

Rwsh_ostream* Substitution_stream::copy_pointer(void) {
  std::abort();} // substitution streams are not used in this way

Rwsh_ostream& Substitution_stream::operator<<(const std::string& r) {
  buffer <<r; 
  return *this;}

Rwsh_ostream& Substitution_stream::operator<<(int r) {
  buffer <<r; 
  return *this;}

Rwsh_ostream& Substitution_stream::operator<<(unsigned int r) {
  buffer <<r; 
  return *this;}

Rwsh_ostream& Substitution_stream::operator<<(double r) {
  buffer <<r; 
  return *this;}

Rwsh_ostream& Substitution_stream::operator<<(struct timeval r) {
  buffer <<r.tv_sec <<"." <<std::setw(6) <<std::setfill('0') <<r.tv_usec;
  return *this;}

bool Substitution_stream::fail(void) {return false;}

int Substitution_stream::fd(void) {
  int fds[2];
  if (pipe(fds)) std::cerr <<"failed pipe with errno " <<errno <<std::endl;
  plumber.proxy_output(fds[0], this);
  plumber.close_on_fork(fds[0]);
  plumber.close_on_wait(fds[1]);
  return fds[1];}

std::string Substitution_stream::str(void) const {
  return ">&{}";}
