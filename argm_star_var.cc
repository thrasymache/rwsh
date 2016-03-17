// The template star_var from the Argm class. It is separated out because gcc
// won't export templates, so it must be included directly in each file where
// it is used.
//
// Copyright (C) 2006-2016 Samuel Newbold

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

template<> std::back_insert_iterator<std::vector<Arg_spec> >
inline Argm::star_var(const std::string& key, unsigned reference_level, 
                std::back_insert_iterator<std::vector<Arg_spec> > res) const {
  int n = std::atoi(key.c_str());
  if (n < 0) n = 0;
  else if (n >= size()) n = size();
  for (const_iterator i = begin()+n; i != end(); ++i) {
    std::string next = *i;
    for (unsigned i = 0; i < reference_level; ++i) next = get_var(next);
    *res++ = Arg_spec(FIXED, 0, 0, 0, -1, 0, next, " ");}
  return res;}

template<class Out>
Out tokenize_words(const std::string& in, Out res) {
  unsigned token_start=0, i=0, nesting=0;
  while (i<in.length() && isspace(in[i])) ++i;           // keep leading space
  for (; i<in.length(); ++i)
    if (in[i] == '(') {if (!nesting++) ++token_start;}
    else if (in[i] == ')') {if (!nesting--)
      throw Signal_argm(Argm::Mismatched_parenthesis, in.substr(0, i+1));}
    else if (!nesting && isspace(in[i])) {
      unsigned end = i;
      while (i<in.length() && isspace(in[i])) ++i;
      if (i == in.length()) end = i;                     // keep trailing space
      if (in[end-1] == ')') *res++ = in.substr(token_start, end-token_start-1);
      else *res++ = in.substr(token_start, end-token_start);
      token_start = i--;}
  if (nesting) throw Signal_argm(Argm::Mismatched_parenthesis, in);
  if (token_start != i)
    if (in[i-1] == ')') *res = in.substr(token_start, i-token_start-1);
    else *res = in.substr(token_start, i-token_start);
  return res;}
