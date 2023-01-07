// Copyright (C) 2007-2023 Samuel Newbold

#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "rwsh_stream.h"
#include "plumber.h"

#include "pipe_stream.h"
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

int Substitution_stream::fd(void) {
  if (fd_v < 3) {
    int fds[2];
    if (pipe(fds)) {
      std::cerr <<"failed pipe with errno " <<errno <<std::endl;
      return -1;}
    in_pipe.late_init(fds[0]);
    fd_v = fds[1];
    plumber.proxy_output(&in_pipe, this);}
  return fd_v;}

std::string Substitution_stream::str(void) const {
  return ">&{}";}
