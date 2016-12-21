// The definition of selection_write and the Entry_pattern class which is
// used to implement selections.
//
// Copyright (C) 2005-2016 Samuel Newbold

#include <dirent.h>
#include <fcntl.h>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "selection.h"
#include "tokenize.cc"

Simple_pattern::Simple_pattern(const std::string& src) {
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

// recursive function to determine matches after initial text. recursivity deals
// with keeping track of current attempted string position in case current
// position fails to match, but subsequent position will succeed.
bool Simple_pattern::find_terms(size_type cur_term, const std::string& s)
        const {
  if (cur_term == terms.size())
    if (unterminated || !s.length()) return true;
    else return false;
  for (std::string::size_type  i = s.find(terms[cur_term]);
       i != std::string::npos; i = s.find(terms[cur_term], i+1))
    if (find_terms(cur_term+1, s.substr(i + terms[cur_term].length())))
      return true;
  return false;}

// returns true if s matches the entry pattern
bool Simple_pattern::match(const std::string& s) const {
  if (s.compare(0, initial.size(), initial)) return false;
  return find_terms(0, s.substr(initial.size()));}

// convert to a string. inverse of constructor.
std::string Simple_pattern::str(void) const {
  std::string result = initial;
  for (std::vector<std::string>::const_iterator i=terms.begin();
       i != terms.end(); ++i)
    result += '*' + *i;
  if (unterminated) result += '*';
  return result;}

Entry_pattern::Entry_pattern(const std::string& src) {
  std::vector<std::string> temp;
  tokenize_strict(src, std::back_inserter(temp),
                  std::bind2nd(std::equal_to<char>(), ' '));
  for (std::vector<std::string>::const_iterator i = temp.begin();
       i != temp.end(); ++i) options.push_back(Simple_pattern(*i));
  only_text = options.size() == 1 && options[0].is_only_text();}

bool Entry_pattern::match(const std::string& s) const {
  for (std::vector<Simple_pattern>::const_iterator i = options.begin();
       i != options.end(); ++i) if (i->match(s)) return true;
  return false;}

std::string Entry_pattern::str(void) const {
  std::string result = options.front().str();
  for (std::vector<Simple_pattern>::const_iterator i = options.begin()+1;
       i != options.end(); ++i) result += ' ' + i->str();
  return result;}

void str_to_entry_pattern_list(const std::string& src,
                               std::list<Entry_pattern>& res) {
  unsigned updir = 0;
  std::vector<std::string> temp;
  tokenize_strict(src, std::back_inserter(temp),
                  std::bind2nd(std::equal_to<char>(), '/'));
  std::vector<std::string>::iterator i = temp.begin();
  if (src.empty());
  else if (*i == "") {
    res.clear();
    res.push_back(std::string());}
  else if (*i == "..")
    if (res.size() > 1) {res.pop_back(); res.pop_back();}
    else {res.clear(); ++updir;}
  else if (*i != ".") {
    if (res.size()) res.pop_back();
    res.push_back(Entry_pattern(*i));}
  for (++i; i != temp.end(); ++i) {
    if (*i == "") res.push_back(Entry_pattern("*"));
    else if (*i == "..")
      if (res.size()) res.pop_back();
      else ++updir;
    else res.push_back(Entry_pattern(*i));}
  for (; updir; --updir) res.push_front(Entry_pattern(std::string("..")));
  if (res.empty()) res.push_back(Entry_pattern("*"));}

std::string entry_pattern_list_to_str(
                               std::list<Entry_pattern>::const_iterator start,
                               std::list<Entry_pattern>::const_iterator end) {
  if (start == end) return std::string();
  std::string result = start->str();
  for (++start; start != end; ++start) result += '/' + start->str();
  if (result == "*") return std::string();
  else return result;}

