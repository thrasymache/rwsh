// Copyright (C) 2007 Samuel Newbold

#include <fstream>
#include <string>

#include "rwsh_stream.h"
#include "file_stream.h"

File_stream_t::File_stream_t(const std::string& file) : dest(file.c_str()) {}

Rwsh_stream_t& File_stream_t::operator<<(const std::string& r) {
  dest <<r;
  return *this;}

Rwsh_stream_t& File_stream_t::operator<<(int r) {
  dest <<r;
  return *this;}

void File_stream_t::flush(void) {dest.flush();}
