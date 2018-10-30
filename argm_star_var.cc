// The template star_var from the Argm class. It is separated out because gcc
// won't export templates, so it must be included directly in each file where
// it is used.
//
// Copyright (C) 2006-2018 Samuel Newbold

// write the strings corresponding to $*
template<class Out>
inline Out Argm::star_var(const std::string& key, unsigned reference_level,
                     Out res) const {
  int n = std::atoi(key.c_str());
  if (n < 0) n = 0;
  else if (n >= argv_v.size()) n = argv_v.size();
  for (const_iterator i = begin()+n; i != end(); ++i) {
    std::string next = *i;
    for (unsigned i = 0; i < reference_level; ++i) next = get_var(next);
    *res++ = next;}
  return res;}
