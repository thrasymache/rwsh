// Copyright (C) 2006, 2007 Samuel Newbold

class Variable_map_t : private std::map<std::string, std::string> {
  typedef std::map<std::string, std::string> Base;
  friend int printenv_bi(const Argv_t& argv);

  unsigned max_nesting_v;
public:
  Variable_map_t(bool root);
  static int dollar_question;
  static bool exit_requested;
  unsigned max_nesting(void) {return max_nesting_v;};

  bool add(const std::string& key, const std::string& value);
  bool exists(const std::string& key) const;
  const std::string& get(const std::string& key);
  int set(const std::string& key, const std::string& value);
  int unset(const std::string& key);
  void append_to_errno(const std::string& value);
  char** export_env(void) const;};

extern int& dollar_question;

inline bool isargvar(const std::string& focus) {
  switch(focus[0]) {
    case '*': case '#': case '0': case '1': case '2': case '3': case '4': 
              case '5': case '6': case '7': case '8': case '9': return true;
    default: return false;}}

