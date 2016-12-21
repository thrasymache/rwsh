// Copyright (C) 2007-2016 Samuel Newbold

#include <string>

#include "rwsh_stream.h"

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

