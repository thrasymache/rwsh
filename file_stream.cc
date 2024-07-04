// Copyright (C) 2007-2023 Samuel Newbold

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "file_stream.h"

File_istream::File_istream(const std::string& name_i) : name(name_i),
    Rwsh_istream(-1) {}

void File_istream::open(void) {
  fd_v = ::open(name.c_str(), O_RDONLY, S_IWUSR|S_IRUSR);
  c_style = fdopen(fd_v, "r");
  if (fd_v == -1 || !c_style)
    throw Exception(Argm::File_open_failure, name);}

File_istream::~File_istream() {
  if(c_style && fclose(c_style))
    std::cerr <<"failed fclose with errno " <<errno <<std::endl;}

int File_istream::fd(void) {
  if (!c_style) open();
  return fd_v;}

Rwsh_istream& File_istream::getline(std::string& dest) {
  if (!c_style) open();
  return read_getline(dest);}

std::string File_istream::str(void) const {
  return "<" + name;}

File_ostream::File_ostream(const std::string& name_i) : name(name_i),
    Rwsh_ostream(-1), c_style(nullptr) {}

void File_ostream::open(void) {
  fd_v = ::open(name.c_str(), O_WRONLY|O_CREAT|O_TRUNC,
                         S_IWUSR|S_IRUSR);
  c_style = fdopen(fd_v, "w");
  if (fd_v == -1 || !c_style)
    throw Exception(Argm::File_open_failure, name);}

File_ostream::~File_ostream() {
  if(c_style && fclose(c_style))
    std::cerr <<"failed fclose with errno " <<errno <<std::endl;}

Rwsh_ostream& File_ostream::operator<<(const std::string& r) {
  if (!c_style) open();
  if (fprintf(c_style, "%s", r.c_str()) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(int r) {
  if (!c_style) open();
  if (fprintf(c_style, "%d", r) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(unsigned int r) {
  if (!c_style) open();
  if (fprintf(c_style, "%u", r) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(double r) {
  if (!c_style) open();
  if (fprintf(c_style, "%f", r) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(struct timeval r) {
  if (!c_style) open();
  if (fprintf(c_style, "%ld.%06ld", (long) r.tv_sec, (long) r.tv_usec) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

int File_ostream::fd(void) {
  if (!c_style) open();
  return fd_v;}

void File_ostream::flush(void) {
  if (!c_style) open();
  if (fflush(c_style) < 0)
    std::cerr <<"failed fflush with errno " <<errno <<std::endl;}

std::string File_ostream::str(void) const {
  return ">" + name;}
