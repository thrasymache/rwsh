// Copyright (C) 2005, 2006 Samuel Newbold

class Simple_pattern {
  bool only_text, unterminated;
  std::string initial;
  std::vector<std::string> terms;

  typedef std::vector<std::string>::size_type size_type;
  bool find_terms(size_type cur_term, const std::string& s) const;
 public:
  Simple_pattern(const std::string& src);
  bool is_only_text(void) const {return only_text;}
  bool match(const std::string& s) const;
  std::string str(void) const;};

class Entry_pattern_t {
  std::vector<Simple_pattern> options;
  bool only_text;
 public:
  Entry_pattern_t(const std::string& src);
  bool is_only_text(void) const {return only_text;}
  bool match(const std::string& s) const;
  std::string str(void) const;};

void str_to_entry_pattern_list(const std::string& src, 
                               std::list<Entry_pattern_t>& res);
std::string entry_pattern_list_to_str(
                               std::list<Entry_pattern_t>::const_iterator start,
                               std::list<Entry_pattern_t>::const_iterator end);

