// Copyright (C) 2007-2018 Samuel Newbold

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "rwsh_stream.h"

void Rwsh_istream::close(void) {
  if (fd_v >= 0) {
    int ret = ::close(fd_v);
    if (ret)
      std::cerr <<"failing close " <<fd_v <<" returned " <<ret <<std::endl;
    c_style = NULL;
    fd_v = -1;}}

Rwsh_istream& Rwsh_istream::cstyle_getline(std::string& dest) {
  char *lineptr = NULL;
  size_t n = 0;
  ssize_t count = ::getline(&lineptr, &n, c_style);
  if (lineptr[count-1] == '\n') lineptr[count-1] = 0;
  else if (count < 0) fail_v = true;
  dest = std::string(lineptr);
  free(lineptr);
  return *this;}

Rwsh_istream& Rwsh_istream::getc_getline(std::string& dest) {
  int c = getc(c_style);
  if (c == EOF) fail_v = true;
  for(; c != EOF && c != '\n'; c = getc(c_style)) dest.push_back(c);
  return *this;}

char *Rwsh_istream::fgets(char *buffer, size_t buffer_size) {
  return ::fgets(buffer, buffer_size, c_style);}

int Rwsh_istream::read(char *buffer, size_t buffer_size) {
  return ::read(fd_v, buffer, buffer_size);}

Rwsh_istream& Rwsh_istream::read_getline(std::string& dest) {
  do {
    for (; pos < unread_end; ++pos)
      if (*pos == '\n') {++pos; return *this;}
      else dest.push_back(*pos);
    pos = unread_end = read_buffer;
    unread_end += ::read(fd(), read_buffer, sizeof(read_buffer));}
  while (unread_end != read_buffer);
  fail_v = true;
  return *this;}

Rwsh_istream_p::Rwsh_istream_p(const Rwsh_istream_p& src) :
  inherited(src.inherited), is_default_v(src.is_default_v) {
  if (src.inherited) implementation = src.implementation;
  else implementation = src.implementation->copy_pointer();}

Rwsh_istream_p Rwsh_istream_p::child_stream(void) const {
  return Rwsh_istream_p(implementation, true, is_default_v);}

Rwsh_istream_p& Rwsh_istream_p::operator=(const Rwsh_istream_p& src) {
  if (!inherited) delete implementation;
  if (src.inherited) {
    implementation = src.implementation;
    inherited = true;
    is_default_v = src.is_default_v;}
  else {
    implementation = src.implementation->copy_pointer();
    inherited = false;
    is_default_v = src.is_default_v;}} // guaranteed to be false

Rwsh_istream_p::~Rwsh_istream_p(void) {
  if (!inherited) delete implementation;}

void Rwsh_ostream::close(void) {
  if (fd_v >= 0) {
    int ret = ::close(fd_v);
    if (ret)
      std::cerr <<"failing close " <<fd_v <<" returned " <<ret <<std::endl;
    c_style = NULL;
    fd_v = -1;}}

Rwsh_ostream_p::Rwsh_ostream_p(const Rwsh_ostream_p& src) :
  inherited(src.inherited), is_default_v(src.is_default_v) {
  if (src.inherited) implementation = src.implementation;
  else implementation = src.implementation->copy_pointer();}

Rwsh_ostream_p Rwsh_ostream_p::child_stream(void) const {
  return Rwsh_ostream_p(implementation, true, is_default_v);}

Rwsh_ostream_p& Rwsh_ostream_p::operator=(const Rwsh_ostream_p& src) {
  if (!inherited) delete implementation;
  if (src.inherited) implementation = src.implementation;
  else implementation = src.implementation->copy_pointer();
  inherited = src.inherited;
  is_default_v = src.is_default_v;}

Rwsh_ostream_p::~Rwsh_ostream_p(void) {
  if (!inherited) delete implementation;}

