// The templates for selection read.
//
// Copyright (C) 2006-2018 Samuel Newbold

// modify partial by treating each element as a directory, and then replacing
// that element with the entries in that directory that match entry_pattern,
// but do not match ignore_pattern.
template<class In>
void partial_match_children(In& partial, const Entry_pattern& entry_pattern,
                            const Entry_pattern& ignore_pattern, bool more) {
  auto first = partial.begin();
  while (first != partial.end()) {
    std::vector<std::string> temp;
    if (first->empty()) read_dir(".", std::back_inserter(temp));
    else read_dir(*first, std::back_inserter(temp));
    for (auto k: temp)                      // each element of directory
      if (entry_pattern.match_with_ignore(ignore_pattern, k)) {
        first = partial.insert(first, *first + k + (more? "/": ""));
        ++first;}                           // point back at dir we are reading
    first = partial.erase(first);}}

// treat src as a selection, add matching files to res.
template<class Out>
void selection_read(const std::string& src, Out res) {
  std::list<Entry_pattern> focus;
  str_to_entry_pattern_list(src, focus);
  Entry_pattern ignore(Variable_map::global_map->get("FIGNORE"));
  std::list<std::string> partial(1);
  for (auto q=focus.begin();
       q!=focus.end() && partial.size(); ++q){// each path step in the selection
    auto k = q;
    bool more = ++k != focus.end();
    if (q->is_only_text())
      for (auto j = partial.begin(); j != partial.end(); ++j) {
        *j += q->str();
        if (more) *j += '/';
        struct stat sb;
        if (stat(j->c_str(), &sb)) {j = partial.erase(j); --j;}}
    else partial_match_children(partial, *q, ignore, more);
    if (!partial.size())
      throw Exception(Argm::Selection_not_found,
                        entry_pattern_list_to_str(focus.begin(), ++q), src);}
  copy(partial.begin(), partial.end(), res);}

