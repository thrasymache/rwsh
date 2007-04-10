// The algorithm templates for tokenizing an input string based upon the 
// predicate p. 
// In each case res must be a container of strings.
//
// Copyright (C) 2005, 2006, 2007 Samuel Newbold

// repeated separators are interpreted as a single separator
// empty input produces no output
template<class Out, class Pred>
Out tokenize(const std::string& in, Out res, Pred p) {
  unsigned token_start=0, i=0;
  for (; i<in.length(); ++i) {
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

// only tokenizes text that is not within brace pairs.
// repeated separators result in empty tokens.
// empty input produces a single empty string
template<class Out, class Pred>
Out tokenize_same_brace_level(const std::string& in, Out res, Pred p) {
  unsigned token_start=0, i=0, brace_level=0;
  for (; i<in.length(); ++i)
    if (in[i] == '{') ++brace_level;
    else if (in[i] == '}') --brace_level;
    else if (!brace_level && p(in[i])) {
      *res++ = in.substr(token_start, i-token_start);
      token_start = i + 1;}
  *res = in.substr(token_start, i-token_start);
  return res;}


