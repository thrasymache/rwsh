// The definition of selection_write and the Entry_pattern class which is
// used to implement selections.
//
// Copyright (C) 2005-2023 Samuel Newbold

#include <dirent.h>
#include <fcntl.h>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "selection.h"
#include "tokenize.cc"

Simple_pattern::Simple_pattern(const std::string& src) {
  std::string::size_type j = src.find_first_of('*');
  initial = std::string(src, 0, j);
  for (auto k = src.find_first_of('*', j+1); k < std::string::npos;
       j = k, k = src.find_first_of('*', j+1))
    terms.push_back(src.substr(j+1, k-j-1));
  only_text = (j == std::string::npos);
  if (j != std::string::npos) last = src.substr(j+1);}

// simplistic version - only checks if initial or final is longer, or if
// remaining terms are contained in competition initial
bool Simple_pattern::isnt_contained(const Simple_pattern& competition) const {
  if (initial.size() > competition.initial.size()) return true;
  if (last.size() > competition.last.size()) return true;
  size_type cur_term = 0;
  for (auto j = initial.size(); cur_term < terms.size(); ++cur_term) {
      j = competition.initial.find(terms[cur_term], j);
      if (j == std::string::npos) break;
      j += terms[cur_term].length();}
  if (cur_term < terms.size()) return true;
  else return false;}

// returns true if s matches the entry pattern
bool Simple_pattern::match(const std::string& s) const {
  auto j = initial.size();
  if (s.substr(0, j) != initial) return false;        // C++20 starts_with
  for (auto cur_term: terms) {
    j = s.find(cur_term, j);
    if (j == std::string::npos) return false;
    j += cur_term.length();}
  auto lstart = s.size() - last.size();
  if ((only_text && j != lstart) ||
      j + last.size() > s.size() ||
      s.substr(lstart) != last) return false;         // C++20 ends_with
  else return true;}

// convert to a string. inverse of constructor.
std::string Simple_pattern::str(void) const {
  std::string result = initial;
  for (auto j: terms) result += '*' + j;
  if (only_text) return result;
  else return result + '*' + last;}

Entry_pattern::Entry_pattern(const std::string& src) {
  std::vector<std::string> temp;
  tokenize_strict(src, std::back_inserter(temp),
                  std::bind2nd(std::equal_to<char>(), ' '));
  for (auto j: temp) options.push_back(Simple_pattern(j));
  only_text = options.size() == 1 && options[0].is_only_text();}

// you still can't encapsulate outer_for in a function....
class OuterContinue : public std::exception {};

bool Entry_pattern::match_with_ignore(const Entry_pattern& ignore_pattern,
                                      const std::string& s) const {
  for (auto competition: options) if (competition.match(s))
    try {
      for (auto j: ignore_pattern.options)
        if (j.match(s) && j.isnt_contained(competition)) throw OuterContinue();
      return true;}
    catch (OuterContinue) {}
  else;
  return false;}

std::string Entry_pattern::str(void) const {
  std::string result = options.front().str();
  for (auto i = options.begin()+1; i != options.end(); ++i)
    result += ' ' + i->str();
  return result;}

void str_to_entry_pattern_list(const std::string& src,
                               std::list<Entry_pattern>& res) {
  unsigned updir = 0;
  std::vector<std::string> temp;
  tokenize_strict(src, std::back_inserter(temp),
                  std::bind2nd(std::equal_to<char>(), '/'));
  auto i = temp.begin();
  if (src.empty());
  else if (*i == "") {
    res.clear();
    res.push_back(*i);}
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

