// The templates for selection read.
//
// Copyright (C) 2006-2019 Samuel Newbold

// replace input by treating each element as a directory, and then replacing
// that element with the entries in that directory that match entry_pattern,
// but do not match ignore_pattern.
template<class In>
In partial_match_children(In& input, const Entry_pattern& entry_pattern,
                          const Entry_pattern& ignore_pattern, bool more) {
  auto output = new std::set<std::string>;
  for (auto dir: *input) {
    std::vector<std::string> temp;
    read_dir(dir.empty()? ".": dir, std::back_inserter(temp));
    for (auto k: temp) if (entry_pattern.match_with_ignore(ignore_pattern, k))
      output->insert(dir + k + (more? "/": ""));}
  delete input;
  return output;}

// treat src as a selection, add matching files to res.
template<class Out>
void selection_read(const std::string& src, Out res) {
  std::list<Entry_pattern> focus;
  str_to_entry_pattern_list(src, focus);
  Entry_pattern ignore(Variable_map::global_map->get("FIGNORE"));
  auto current = new std::set<std::string>({""});
  for (auto q = focus.begin(); q != focus.end() && current->size();
       ++q) { // each path step in the selection
    auto k = q;
    bool more = ++k != focus.end();
    if (q->is_only_text()) {
      auto next = new std::set<std::string>;
      for (auto result: *current) {
        result += q->str() + (more? "/": "");
        struct stat sb;
        if (!stat(result.c_str(), &sb)) next->insert(result);}
      delete current;
      current = next;}
    else current = partial_match_children(current, *q, ignore, more);
    if (!current->size())
      throw Exception(Argm::Selection_not_found,
                      entry_pattern_list_to_str(focus.begin(), ++q), src);}
  copy(current->begin(), current->end(), res);
  delete current;}

