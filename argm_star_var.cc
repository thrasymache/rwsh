// The template star_var from the Argm class. It is separated out because gcc
// won't export templates, so it must be included directly in each file where
// it is used.
//
// Copyright (C) 2006-2018 Samuel Newbold

// constructor of Argm from a pair of iterators
template <class String_it>
inline Argm::Argm(String_it first_string, String_it last_string,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i,
       Rwsh_ostream_p error_i) :
  Base(first_string, last_string),
  argfunction_v(argfunction_i->copy_pointer()),
  input(input_i), output(output_i), error(error_i),
  parent_map_v(parent_map_i) {
    argc_v = Base::size();};

// constructor of Argm from an initial argument and a  pair of iterators
template <class String_it>
inline Argm::Argm(const std::string& first_string,
       String_it second_string, String_it last_string,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i,
       Rwsh_ostream_p error_i) :
  Base(second_string, last_string),
  argfunction_v(argfunction_i->copy_pointer()),
  input(input_i), output(output_i), error(error_i),
  parent_map_v(parent_map_i) {
    Base::insert(Base::begin(), first_string);
    argc_v = Base::size();};

// write the strings corresponding to $*
template<class Out>
inline Out Argm::star_var(const std::string& key, unsigned reference_level,
                     Out res) const {
  int n = std::atoi(key.c_str());
  if (n < 0) n = 0;
  else if (n >= size()) n = size();
  for (const_iterator i = begin()+n; i != end(); ++i) {
    std::string next = *i;
    for (unsigned i = 0; i < reference_level; ++i) next = get_var(next);
    *res++ = next;}
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
