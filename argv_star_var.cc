// The template star_var from the Argv_t class. It is separated out because gcc
// won't export templates, so it must be included directly in each file where
// it is used.
//
// Copyright (C) 2005, 2006 Samuel Newbold

// write the strings corresponding to $*
template<class Out>
Out Argv_t::star_var(const std::string& key, Out res) const {
  int n = std::atoi(key.c_str());
  if (n < 0) n = 0;
  else if (n >= size()) n = size();
  for (const_iterator i = begin()+n; i != end(); ++i) *res++ = *i;
  return res;}

