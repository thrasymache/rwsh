// Copyright (C) 2018 Samuel Newbold

#include <cstdlib>
#include <list>
#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "pipe_stream.h"

Pipe_istream::Pipe_istream(int fd_i) : Rwsh_istream(fd_i) {
  if(fd_v < 3) c_style = nullptr;
  else {
    c_style = fdopen(fd_v, "r");
    if (!c_style) throw Exception(Argm::File_open_failure, fd_v);}} // untested

void Pipe_istream::late_init(int fd_i) {
  if(fd_v < 3) {
    fd_v = fd_i;
    c_style = fdopen(fd_v, "r");
    if (!c_style) throw Exception(Argm::File_open_failure, fd_v);}
  else throw Exception(Argm::Internal_error, fd_v, fd_i);}

Rwsh_istream* Pipe_istream::copy_pointer(void) {std::abort();}; // not needed

Rwsh_istream& Pipe_istream::getline(std::string& dest) {
  return read_getline(dest);}

std::string Pipe_istream::str(void) const {return "|";}

