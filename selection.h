// Copyright (C) 2005, 2006 Samuel Newbold

class Entry_pattern_t {
  typedef std::vector<std::string>::size_type size_type;
  bool only_text, unterminated;
  std::string initial;
  std::vector<std::string> terms;
  mutable std::string string_v;
  bool find_terms(size_type cur_term, const std::string& s) const;
 public:
  Entry_pattern_t(const std::string& src);
  bool is_only_text(void) const {return only_text;}
  const std::string& initial_text(void) const {return initial;}
  bool match(const std::string& s) const;
  const std::string& str(void) const;};

void str_to_entry_pattern_vector(const std::string& src, 
                          std::vector<Entry_pattern_t>& res);
std::string entry_pattern_vector_to_str(
                                       const std::vector<Entry_pattern_t>& src);

