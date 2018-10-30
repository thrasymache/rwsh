// The algorithm templates for tokenizing an input string based upon the
// predicate p.
// In each case res must be a container of strings.
//
// Copyright (C) 2005-2018 Samuel Newbold

// repeated separators are interpreted as a single separator
// empty input produces no output
template<class Out, class Pred>
Out tokenize(const std::string& in, Out res, Pred p) {
  unsigned token_start=-1, i=0;
  while (i<in.length() && p(in[i])) ++i;
  for (token_start = i; i<in.length(); ++i) {
    if (p(in[i])) {
      *res++ = in.substr(token_start, i-token_start);
      while (i<in.length() && p(in[i])) ++i;
      token_start = i + (i>=in.length());}}        //dealing with end of line:
  if (token_start != i) *res = in.substr(token_start, i-token_start);
  return res;}
// dealing with end of line: adding the condition (i>=in.length()) means that if
// the while loop found a token character, then token_start will be equal to
// it. If the loop ended because the string was exhausted, then i addresses the
// element one past the end of the string, and token_start will represent a
// character two past the end, just as i will after it is incremented. Thus
// the test for token_start != i will tell if there is in fact a token

// repeated separators result in empty tokens
// empty input produces a single empty string
template<class Out, class Pred>
Out tokenize_strict(const std::string& in, Out res, Pred p) {
  unsigned token_start=0, i=0;
  for (; i<in.length(); ++i)
    if (p(in[i])) {
      *res++ = in.substr(token_start, i-token_start);
      token_start = i + 1;}
  *res = in.substr(token_start, i-token_start);
  return res;}

namespace {
std::string substr_word(const std::string& in, unsigned start, unsigned end) {
  if (in[start] == '(' && in[end-1] == ')')
    return in.substr(start+1, end-start-2);
  else return in.substr(start, end-start);}
} // end unnamed namespace

template<class Out>
Out tokenize_words(const std::string& in, Out res) {
  unsigned j=0, nesting=0;
  while (j < in.length() && isspace(in[j])) ++j;
  unsigned token_start = j;                              // drop leading space
  for (; j<in.length(); ++j)
    if (isspace(in[j])) {
      *res++ = substr_word(in, token_start, j);
      while (j < in.length() && isspace(in[j])) ++j;
      token_start = j--;}                                // drop leading space
    else if (in[j] == ')')
      throw Exception(Argm::Mismatched_parenthesis, in.substr(0, j+1));
    else if (in[j] == '(') for (nesting++; nesting && ++j<in.length();)
      if (in[j] == '(') nesting++;
      else if (in[j] == ')') nesting--;
      else; // step through nested non-parentheses
    else;   // step through non-nested printing non-parentheses
  if (nesting) throw Exception(Argm::Mismatched_parenthesis, in);
  if (token_start != j) *res++ = substr_word(in, token_start, j);
  return res;}
