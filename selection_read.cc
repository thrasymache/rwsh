// The templates for selection read.
//
// Copyright (C) 2006 Samuel Newbold

// modify partial by treating each element as a directory, and then replacing
// that element with the entries in that directory that match entry_pattern,
// but do not match ignore_pattern.
template<class In> 
void partial_match_children(In& partial, const Entry_pattern_t& entry_pattern,
                            const Entry_pattern_t& ignore_pattern) {
  typename In::iterator first = partial.begin();
  while (first != partial.end()) {
    std::vector<std::string> temp;
    read_dir(*first, std::back_inserter(temp));
    for (std::vector<std::string>::iterator k=temp.begin(); k!=temp.end(); 
         ++k) { // each element of directory
      if (ignore_pattern.match(*k)) continue;
      if (entry_pattern.match(*k)) {
        first = partial.insert(first, *first + '/' + *k);
        ++first;}} // point back at dir we are reading
    first = partial.erase(first);}}

// treat src as a selection, add matching files to res.
template<class Out>
void selection_read(const std::string& src, Out res) {
  std::vector<Entry_pattern_t> focus;
  str_to_entry_pattern_vector(src, focus);
  Entry_pattern_t ignore(vars->get("FIGNORE"));
  std::list<std::string> partial(1);
  for (std::vector<Entry_pattern_t>::iterator i=focus.begin(); 
       i!=focus.end() && partial.size(); ++i){// each path step in the selection
    if (i->is_only_text())
      for (std::list<std::string>::iterator j=partial.begin();
           j!=partial.end(); ++j) {
        *j += '/' + i->initial_text();
        struct stat sb;
        if (stat(j->c_str(), &sb)) {j = partial.erase(j); --j;}}
    else partial_match_children(partial, *i, ignore);
    if (!partial.size()) {
      std::string error_string = "rwsh.selection_not_found ";
      for (std::vector<Entry_pattern_t>::iterator j=focus.begin(); j!=i+1; ++j)
        error_string += "/" + j->str();
      error_string += " ";
      for (std::vector<Entry_pattern_t>::iterator j=focus.begin(); 
           j!=focus.end(); ++j)
        error_string += "/" + j->str();
      Argv_t argv(error_string);
      executable_map[argv](argv);}}
  copy(partial.begin(), partial.end(), res);}

