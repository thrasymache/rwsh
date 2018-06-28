// Copyright (C) 2008-2018 Samuel Newbold

#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "default_stream.h"

Default_istream::Default_istream(int fd_i) : Rwsh_istream(fd_i) {
  switch (fd_v) {
    case 0:  c_style = stdin; break;
    default: std::abort();}}

Rwsh_istream* Default_istream::copy_pointer(void) {std::abort();}; // not needed

int Default_istream::fd(void) {return fd_v;}

Rwsh_istream& Default_istream::getline(std::string& dest) {
  if (readline_enabled) return readline_getline(dest);
  else return read_getline(dest);}

Rwsh_istream& Default_istream::readline_getline(std::string& dest) {
  char *line = readline("");
  if (!line) {
    fail_v = true;
    return *this;
  }
  add_history(line);
  dest = std::string(line);
  free(line);
  return *this;}

std::string Default_istream::str(void) const {return "<&1";}

Default_ostream::Default_ostream(int fd_i) : Rwsh_ostream(fd_i) {
  switch (fd_v) {
    case 1: cpp_style = &std::cout; c_style = stdout; break;
    case 2: cpp_style = &std::cerr; c_style = stderr; break;
    default: std::abort();}}

Rwsh_ostream* Default_ostream::copy_pointer(void) {std::abort();}; // not needed

Rwsh_ostream& Default_ostream::operator<<(const std::string& r) {
  *cpp_style <<r;
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(int r) {
  *cpp_style <<r;
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(unsigned int r) {
  *cpp_style <<r;
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(double r) {
  *cpp_style <<r;
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(struct timeval r) {
  *cpp_style <<r.tv_sec <<"."
                  <<std::setw(6) <<std::setfill('0') <<r.tv_usec;
  return *this;}

int Default_ostream::fd(void) {return fd_v;}

void Default_ostream::flush(void) {cpp_style->flush();}

std::string Default_ostream::str(void) const {return ">&0";}

