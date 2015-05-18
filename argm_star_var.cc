// The template star_var from the Argm class. It is separated out because gcc
// won't export templates, so it must be included directly in each file where
// it is used.
//
// Copyright (C) 2006-2015 Samuel Newbold

// constructor of Argm from a pair of iterators
template <class String_it> 
inline Argm::Argm(String_it first_string, String_it last_string,
       Function* argfunction_i, 
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i,
       Rwsh_ostream_p error_i) :
  Base(first_string, last_string),
  argfunction_v(argfunction_i->copy_pointer()), 
  input(input_i), output(output_i), error(error_i),
  parent_map(Variable_map::global_map) {};

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
    *res++ = Arg_spec(FIXED, 0, 0, 0, -1, 0, next);}
  return res;}

