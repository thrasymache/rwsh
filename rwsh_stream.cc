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

int Default_stream_t::fileno(void) {return 1;}

void Default_stream_t::flush(void) {std::cout.flush();}

std::string Default_stream_t::str(void) const {return "";}

Rwsh_stream_t& Inheirited_stream_t::operator<<(const std::string& r) {
  parent <<r;
  return *this;}

Rwsh_stream_t& Inheirited_stream_t::operator<<(int r) {
  parent <<r;
  return *this;}

int Inheirited_stream_t::fileno(void) {return parent.fileno();}

void Inheirited_stream_t::flush(void) {parent.flush();}

std::string Inheirited_stream_t::str(void) const {return parent.str();}
