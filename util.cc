// Utility function to find the next close brace at the same nesting level. 
//
// Copyright (C) 2006 Samuel Newbold

#include <iostream>
#include <string>

#include "util.h"

std::string::size_type find_close_brace(const std::string& focus,
                                        std::string::size_type i) {
  unsigned nesting = 1;
  while (nesting && (i = focus.find_first_of("{}", i+1)) != std::string::npos) {
    if (focus[i] == '{') ++nesting;
    else --nesting;}
  return i;}

