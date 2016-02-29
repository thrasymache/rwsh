// Copyright (C) 2006-2015 Samuel Newbold

extern const char* WSPACE;

class Variable_map : private std::map<std::string, std::string> {
  typedef std::map<std::string, std::string> Base;
  Variable_map* parent;

  static unsigned max_nesting_v;
public:

  typedef Base::iterator iterator;
  typedef Base::const_iterator const_iterator;
  static Variable_map *global_map;

  Variable_map(Variable_map* parent);
  static int dollar_question;
  static bool exit_requested;
  unsigned max_nesting(void) {return max_nesting_v;};

  const_iterator begin(void) const {return Base::begin();};
  const_iterator end(void) const {return Base::end();};
  iterator begin(void) {return Base::begin();};
  iterator end(void) {return Base::end();};
  unsigned size(void) const {return Base::size();};
  void append_word_locally(const std::string& key, const std::string& value);
  void append_word_if_exists(const std::string& key, const std::string& value);
  void local_or_append_word(const std::string& key, const std::string& value);
  bool exists(const std::string& key) const;
  const std::string& get(const std::string& key);
  int global(const std::string& key, const std::string& value);
  int local(const std::string& key, const std::string& value);
  void set(const std::string& key, const std::string& value);
  int unset(const std::string& key);
  template <class In>
      char** copy_to_char_star_star(In first, In last, char** res);
  char** export_env(void);};

inline bool isargvar(const std::string& focus) {
  switch(focus[0]) {
    case '*': case '#': case '0': case '1': case '2': case '3': case '4': 
              case '5': case '6': case '7': case '8': case '9': return true;
    default: return false;}}

