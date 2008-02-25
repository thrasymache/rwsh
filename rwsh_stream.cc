// Copyright (C) 2007 Samuel Newbold

#include <assert.h>
#include <iostream>
#include <string>

#include "rwsh_stream.h"

struct Default_stream_t : public Rwsh_stream_t {
  virtual Rwsh_stream_t* copy_pointer(void) {assert(0);}; // not needed
  virtual Rwsh_stream_t& operator<<(const std::string& r);
  virtual Rwsh_stream_t& operator<<(int r);
  virtual int fileno(void);
  virtual void flush(void);
  virtual std::string str(void) const;};

Rwsh_stream_t& Default_stream_t::operator<<(const std::string& r) {
  std::cout <<r; 
  return *this;}

Rwsh_stream_t& Default_stream_t::operator<<(int r) {
  std::cout <<r; 
  return *this;}

int Default_stream_t::fileno(void) {return 1;}

void Default_stream_t::flush(void) {std::cout.flush();}

std::string Default_stream_t::str(void) const {return "";}

Default_stream_t default_stream;

Rwsh_stream_p::Rwsh_stream_p(void) : 
  implementation(&default_stream), inherited(true) {};

Rwsh_stream_p::Rwsh_stream_p(const Rwsh_stream_p& src) { 
  if (src.inherited) {
    implementation = src.implementation;
    inherited = true;}
  else {
    implementation = src.implementation->copy_pointer();
    inherited = false;}}

Rwsh_stream_p Rwsh_stream_p::child_stream(void) const {
  return Rwsh_stream_p(implementation, true);}

Rwsh_stream_p& Rwsh_stream_p::operator=(const Rwsh_stream_p& src) {
  if (!inherited) delete implementation;
  if (src.inherited) {
    implementation = src.implementation;
    inherited = true;}
  else {
    implementation = src.implementation->copy_pointer();
    inherited = false;}}

Rwsh_stream_p::~Rwsh_stream_p(void) {
  if (!inherited) delete implementation;}

bool Rwsh_stream_p::is_default(void) const {
  return implementation == &default_stream;}

