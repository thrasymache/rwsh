// Copyright (C) 2007-2016 Samuel Newbold

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "file_stream.h"

File_istream::File_istream(const std::string& name_i) : name(name_i),
    file_descriptor(-1), dest(NULL), fail_v(false) {}

void File_istream::open(void) {
  file_descriptor = ::open(name.c_str(), O_RDONLY, S_IWUSR|S_IRUSR);
  dest = fdopen(file_descriptor, "r");
  if (file_descriptor == -1 || !dest)
    throw Exception(Argm::File_open_failure, name);}

File_istream::~File_istream() {
  if(dest) if (fclose(dest))
    std::cerr <<"failed fclose with errno " <<errno <<std::endl;}

bool File_istream::fail(void) {return fail_v;}

Rwsh_istream& File_istream::getline(std::string& dest_str) {
  if (!dest) open();
  int c = getc(dest);
  for(; c != EOF && c != '\n'; c = getc(dest)) dest_str.push_back(c);
  if (c == EOF) fail_v = true;
  return *this;}

int File_istream::fd(void) {
  if (!dest) open();
  return file_descriptor;}

std::string File_istream::str(void) const {
  return "<" + name;}

File_ostream::File_ostream(const std::string& name_i) : name(name_i),
    file_descriptor(-1), dest(NULL) {}

void File_ostream::open(void) {
  file_descriptor = ::open(name.c_str(), O_WRONLY|O_CREAT|O_TRUNC,
                         S_IWUSR|S_IRUSR);
  dest = fdopen(file_descriptor, "w");
  if (file_descriptor == -1 || !dest)
    throw Exception(Argm::File_open_failure, name);}

File_ostream::~File_ostream() {
  if(dest) if (fclose(dest))
    std::cerr <<"failed fclose with errno " <<errno <<std::endl;}

Rwsh_ostream& File_ostream::operator<<(const std::string& r) {
  if (!dest) open();
  if (fprintf(dest, "%s", r.c_str()) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(int r) {
  if (!dest) open();
  if (fprintf(dest, "%d", r) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(unsigned int r) {
  if (!dest) open();
  if (fprintf(dest, "%u", r) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(double r) {
  if (!dest) open();
  if (fprintf(dest, "%f", r) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

Rwsh_ostream& File_ostream::operator<<(struct timeval r) {
  if (!dest) open();
  if (fprintf(dest, "%ld.%06ld", (long) r.tv_sec, (long) r.tv_usec) < 0)
    std::cerr <<"failed fprintf with errno " <<errno <<std::endl;
  return *this;}

bool File_ostream::fail(void) {
  if (!dest) open();
  int ret = ferror(dest);
  return (bool) ret;}

int File_ostream::fd(void) {
  if (!dest) open();
  return file_descriptor;}

void File_ostream::flush(void) {
  if (!dest) open();
  if (fflush(dest) < 0)
    std::cerr <<"failed fflush with errno " <<errno <<std::endl;}

std::string File_ostream::str(void) const {
  return ">" + name;}
