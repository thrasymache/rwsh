// Copyright (C) 2007 Samuel Newbold

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "file_stream.h"

File_istream_t::File_istream_t(const std::string& name_i) : name(name_i),
    file_descriptor(-1), dest(NULL), fail_v(false) {}

void File_istream_t::open(void) {
  file_descriptor = ::open(name.c_str(), O_RDONLY, S_IWUSR|S_IRUSR);
  dest = fdopen(file_descriptor, "r");
  if (!dest) throw File_open_failure_t(name);}

File_istream_t::~File_istream_t() {
  if(dest) if (fclose(dest))
    std::cerr <<"failed fclose with errno " <<errno <<std::endl;}

bool File_istream_t::fail(void) {return fail_v;}

Rwsh_istream_t& File_istream_t::getline(std::string& dest_str) {
  if (!dest) open();
  int c = getc(dest);
  for(; c != EOF && c != '\n'; c = getc(dest)) dest_str.push_back(c);
  if (c == EOF) fail_v = true;
  return *this;}

int File_istream_t::fileno(void) {
  if (!dest) open();
  return file_descriptor;}

std::string File_istream_t::str(void) const {
  return "<" + name;}

File_ostream_t::File_ostream_t(const std::string& name_i) : name(name_i),
    file_descriptor(-1), dest(NULL) {}

void File_ostream_t::open(void) {
  file_descriptor = ::open(name.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 
                         S_IWUSR|S_IRUSR);
  dest = fdopen(file_descriptor, "w");
  if (!dest) throw File_open_failure_t(name);}

File_ostream_t::~File_ostream_t() {
  if(dest) if (fclose(dest))
    std::cerr <<"failed fclose with errno " <<errno <<std::endl;}

Rwsh_ostream_t& File_ostream_t::operator<<(const std::string& r) {
  if (!dest) open();
  if (fprintf(dest, "%s", r.c_str()) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream_t& File_ostream_t::operator<<(int r) {
  if (!dest) open();
  if (fprintf(dest, "%d", r) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream_t& File_ostream_t::operator<<(struct timeval r) {
  if (!dest) open();
  if (fprintf(dest, "%d.%06d", r.tv_sec, r.tv_usec) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}
  
bool File_ostream_t::fail(void) {
  if (!dest) open();
  int ret = ferror(dest);
  return (bool) ret;}

int File_ostream_t::fileno(void) {
  if (!dest) open();
  return file_descriptor;}

void File_ostream_t::flush(void) {
  if (!dest) open();
  if (fflush(dest) < 0)
    std::cerr <<"failed fflush with errno " <<errno <<std::endl;}

std::string File_ostream_t::str(void) const {
  return ">" + name;}
