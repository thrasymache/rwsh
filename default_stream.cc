// Copyright (C) 2008 Samuel Newbold

#include <assert.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "default_stream.h"

Default_istream_t::Default_istream_t(int fileno_i) : fileno_v(fileno_i) {
  switch (fileno_v) {
    case 0: implementation = &std::cin; break;
    default: assert(0);}}

Rwsh_istream_t* Default_istream_t::copy_pointer(void) {assert(0);}; // not needed

bool Default_istream_t::fail(void) {return implementation->fail();}

Rwsh_istream_t& Default_istream_t::getline(std::string& dest) {
  std::getline(*implementation, dest);
  return *this;}

int Default_istream_t::fileno(void) {return fileno_v;}

std::string Default_istream_t::str(void) const {return "";}

Default_ostream_t::Default_ostream_t(int fileno_i) : fileno_v(fileno_i) {
  switch (fileno_v) {
    case 1: implementation = &std::cout; break;
    case 2: implementation = &std::cerr; break;
    default: assert(0);}}

Rwsh_ostream_t* Default_ostream_t::copy_pointer(void) {assert(0);}; // not needed

Rwsh_ostream_t& Default_ostream_t::operator<<(const std::string& r) {
  *implementation <<r; 
  return *this;}

Rwsh_ostream_t& Default_ostream_t::operator<<(int r) {
  *implementation <<r; 
  return *this;}

Rwsh_ostream_t& Default_ostream_t::operator<<(struct timeval r) {
  *implementation <<r.tv_sec <<"."
                  <<std::setw(6) <<std::setfill('0') <<r.tv_usec;
  return *this;}

bool Default_ostream_t::fail(void) {return implementation->fail();}

int Default_ostream_t::fileno(void) {return fileno_v;}

void Default_ostream_t::flush(void) {implementation->flush();}

std::string Default_ostream_t::str(void) const {return "";}

