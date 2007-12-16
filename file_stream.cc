// Copyright (C) 2007 Samuel Newbold

#include <stdio.h>
#include <fcntl.h>
#include <string>
#include <vector>

#include "argv.h"
#include "rwsh_stream.h"
#include "file_stream.h"

File_stream_t::File_stream_t(const std::string& name_i) : name(name_i),
    file_descriptor(-1), dest(NULL) {}

void File_stream_t::open(void) {
  file_descriptor = ::open(name.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 
                         S_IWUSR|S_IRUSR);
  dest = fdopen(file_descriptor, "w");
  if (!dest) throw File_open_failure_t(name);}

File_stream_t::~File_stream_t() {
  if(dest) fclose(dest);}

Rwsh_stream_t& File_stream_t::operator<<(const std::string& r) {
  if (!dest) open();
  fprintf(dest, "%s", r.c_str());
  return *this;}

Rwsh_stream_t& File_stream_t::operator<<(int r) {
  if (!dest) open();
  fprintf(dest, "%d", r);
  return *this;}

int File_stream_t::fileno(void) {
  if (!dest) open();
  return file_descriptor;}

void File_stream_t::flush(void) {
  if (!dest) open();
  fflush(dest);}

std::string File_stream_t::str(void) const {
  return ">" + name;}
