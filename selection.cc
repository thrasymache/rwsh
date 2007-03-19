// The definition of selection_write and the Entry_pattern_t class which is 
// used to implement selections.
//
// Copyright (C) 2005, 2006 Samuel Newbold

#include <dirent.h>
#include <fcntl.h>
#include <functional>
#include <iterator>
#include <list>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "argv.h"
#include "selection.h"
#include "tokenize.cc"
#include "util.h"

Entry_pattern_t::Entry_pattern_t(const std::string& src) {
  std::string::size_type j = src.find_first_of('*');
  if (j == std::string::npos) {
    only_text = true;
    initial = src;
    unterminated = false;}
  else {
    only_text = false;
    initial = std::string(src, 0, j);
    unterminated = (*src.rbegin() == '*');
    do {
      std::string::size_type k = src.find_first_of('*', j+1);
      if (k > j+1 && j+1 != src.length()) 
        terms.push_back(src.substr(j+1, k-j-1));
      j = k;}
    while (j < std::string::npos);}}

// recursive function to determine matches after initial text 
bool Entry_pattern_t::find_terms(size_type cur_term, const std::string& s) 
        const {
  if (cur_term == terms.size())
    if (unterminated || !s.length()) return true;
    else return false;
  std::string::size_type i = 0; 
  do {
    i = s.find(terms[cur_term], i);
    if (i == std::string::npos) return false;
    i += terms[cur_term].length();
    if (find_terms(cur_term+1, s.substr(i))) return true;}
  while (i < s.length() - terms[cur_term].length());
  return false;}

// returns true if s matches the entry pattern
bool Entry_pattern_t::match(const std::string& s) const {
  if (s.compare(0, initial.size(), initial)) return false;
  return find_terms(0, s.substr(initial.size()));}

// convert to a string. inverse of constructor.
const std::string& Entry_pattern_t::str(void) const {
  string_v = initial;
  for (std::vector<std::string>::const_iterator i=terms.begin(); 
       i != terms.end(); ++i)
    string_v += '*' + *i;
  if (unterminated) string_v += '*';
  return string_v;}

void str_to_entry_pattern_vector(const std::string& src, 
                                 std::vector<Entry_pattern_t>& res) {
  std::vector<std::string> temp;
  tokenize_strict(src, std::back_inserter(temp), 
                  std::bind2nd(std::equal_to<char>(), '/'));
  std::vector<std::string>::iterator i = temp.begin();
  if (!i->size()) ++i;
  else if (res.size()) res.pop_back();
  for (; i != temp.end(); ++i) {
    if (*i == "") 
      if (i+1 == temp.end()) res.push_back(Entry_pattern_t("*"));
      else res.clear();
    else if (*i == "..") {if (res.size()) res.pop_back();}
    else res.push_back(Entry_pattern_t(*i));}}

std::string entry_pattern_vector_to_str(
                                      const std::vector<Entry_pattern_t>& src) {
  std::string result;
  for (std::vector<Entry_pattern_t>::const_iterator i=src.begin();
       i != src.end(); ++i) result += '/' + i->str();
  return result;}

