// Copyright (C) 2008 Samuel Newbold

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "default_stream.h"

Default_istream::Default_istream(int fd_i) : fd_v(fd_i) {
  switch (fd_v) {
    case 0: implementation = &std::cin; break;
    default: abort();}}

Rwsh_istream* Default_istream::copy_pointer(void) {abort();}; // not needed

bool Default_istream::fail(void) {return implementation->fail();}

Rwsh_istream& Default_istream::getline(std::string& dest) {
  std::getline(*implementation, dest);
  return *this;}

int Default_istream::fd(void) {return fd_v;}

std::string Default_istream::str(void) const {return "<&1";}

Default_ostream::Default_ostream(int fd_i) : fd_v(fd_i) {
  switch (fd_v) {
    case 1: implementation = &std::cout; break;
    case 2: implementation = &std::cerr; break;
    default: abort();}}

Rwsh_ostream* Default_ostream::copy_pointer(void) {abort();}; // not needed

Rwsh_ostream& Default_ostream::operator<<(const std::string& r) {
  *implementation <<r; 
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(int r) {
  *implementation <<r; 
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(unsigned int r) {
  *implementation <<r; 
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(double r) {
  *implementation <<r; 
  return *this;}

Rwsh_ostream& Default_ostream::operator<<(struct timeval r) {
  *implementation <<r.tv_sec <<"."
                  <<std::setw(6) <<std::setfill('0') <<r.tv_usec;
  return *this;}

bool Default_ostream::fail(void) {return implementation->fail();}

int Default_ostream::fd(void) {return fd_v;}

void Default_ostream::flush(void) {implementation->flush();}

std::string Default_ostream::str(void) const {return ">&0";}

