// Copyright (C) 2005-2018 Samuel Newbold

// a set of fixed text terms separated by wildcards to match directory entries
class Simple_pattern {
  bool only_text;
  std::string initial;
  std::string last;
  std::vector<std::string> terms;

  typedef std::vector<std::string>::size_type size_type;
 public:
  Simple_pattern(const std::string& src);
  bool is_only_text(void) const {return only_text;}
  bool isnt_contained(const Simple_pattern& competition) const;
  bool match(const std::string& s) const;
  std::string str(void) const;};

// a set of unrelated Simple_pattern options, to match directory entries
class Entry_pattern {
  std::vector<Simple_pattern> options;
  bool only_text;
 public:
  Entry_pattern(const std::string& src);
  bool is_only_text(void) const {return only_text;}
  bool match_with_ignore(const Entry_pattern& ignore_pattern,
                         const std::string& s) const;
  bool specific_match(const std::string& s,
                      const Simple_pattern& competition) const;
  std::string str(void) const;};

void str_to_entry_pattern_list(const std::string& src,
                               std::list<Entry_pattern>& res);
std::string entry_pattern_list_to_str(
                               std::list<Entry_pattern>::const_iterator start,
                               std::list<Entry_pattern>::const_iterator end);

