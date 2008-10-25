// The templates for selection read.
//
// Copyright (C) 2006, 2007 Samuel Newbold

// modify partial by treating each element as a directory, and then replacing
// that element with the entries in that directory that match entry_pattern,
// but do not match ignore_pattern.
template<class In> 
void partial_match_children(In& partial, const Entry_pattern_t& entry_pattern,
                            const Entry_pattern_t& ignore_pattern, bool more) {
  typename In::iterator first = partial.begin();
  while (first != partial.end()) {
    std::vector<std::string> temp;
    if (first->empty()) read_dir(".", std::back_inserter(temp));
    else read_dir(*first, std::back_inserter(temp));
    for (std::vector<std::string>::iterator k=temp.begin(); k!=temp.end(); 
         ++k) { // each element of directory
      if (ignore_pattern.match(*k)) continue;
      if (entry_pattern.match(*k)) {
        first = partial.insert(first, *first + *k + (more? "/": ""));
        ++first;}} // point back at dir we are reading
    first = partial.erase(first);}}

// treat src as a selection, add matching files to res.
template<class Out>
void selection_read(const std::string& src, Out res) {
  extern Variable_map_t* vars;
  std::list<Entry_pattern_t> focus;
  str_to_entry_pattern_list(src, focus);
  Entry_pattern_t ignore(vars->get("FIGNORE"));
  std::list<std::string> partial(1);
  for (std::list<Entry_pattern_t>::const_iterator i=focus.begin(); 
       i!=focus.end() && partial.size(); ++i){// each path step in the selection
    std::list<Entry_pattern_t>::const_iterator k = i; 
    bool more = ++k != focus.end();
    if (i->is_only_text())
      for (std::list<std::string>::iterator j=partial.begin();
           j!=partial.end(); ++j) {
        *j += i->initial_text();
        if (more) *j += '/';
        struct stat sb;
        if (stat(j->c_str(), &sb)) {j = partial.erase(j); --j;}}
    else partial_match_children(partial, *i, ignore, more);
    if (!partial.size()) {
      Argv_t argv;
      argv.push_back("rwsh.selection_not_found");
      std::list<Entry_pattern_t>::const_iterator j = i;
      argv.push_back(entry_pattern_list_to_str(focus.begin(), ++j));
      argv.push_back(src);
      executable_map.run(argv);}}
  copy(partial.begin(), partial.end(), res);}

