// The template star_var from the Argv_t class. It is separated out because gcc
// won't export templates, so it must be included directly in each file where
// it is used.
//
// Copyright (C) 2006 Samuel Newbold

// constructor of Argv from a pair of iterators
template <class String_it> 
inline Argv_t::Argv_t(String_it first_string, String_it last_string,
       Function_t* argfunction_i, 
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i,
       Rwsh_ostream_p error_i) :
  Base(first_string, last_string),
  argfunction_v(argfunction_i->copy_pointer()), 
  input(input_i), output(output_i), error(error_i) {};

// write the strings corresponding to $*
template<class Out>
inline Out Argv_t::star_var(const std::string& key, unsigned reference_level, 
                     Out res) const {
  int n = std::atoi(key.c_str());
  if (n < 0) n = 0;
  else if (n >= size()) n = size();
  for (const_iterator i = begin()+n; i != end(); ++i) {
    std::string next = *i;
    for (unsigned i = 0; i < reference_level; ++i) next = get_var(next);
    *res++ = next;}
  return res;}

template<> std::back_insert_iterator<std::vector<Arg_spec_t> >
inline Argv_t::star_var(const std::string& key, unsigned reference_level, 
                std::back_insert_iterator<std::vector<Arg_spec_t> > res) const {
  int n = std::atoi(key.c_str());
  if (n < 0) n = 0;
  else if (n >= size()) n = size();
  for (const_iterator i = begin()+n; i != end(); ++i) {
    std::string next = *i;
    for (unsigned i = 0; i < reference_level; ++i) next = get_var(next);
    *res++ = Arg_spec_t(FIXED, 0, 0, 0, 0, next);}
  return res;}

