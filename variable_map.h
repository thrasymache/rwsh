// Copyright (C) 2006 Samuel Newbold

class Variable_map_t : private std::map<std::string, std::string> {
  typedef std::map<std::string, std::string> Base;
  friend int printenv_bi(const Argv_t& argv);
public:
  Variable_map_t(void) : max_nesting(0) {};
  const std::string& get(const std::string& key);
  void set(const std::string& key, const std::string& value);
  char** export_env(void);

  static int dollar_question;
  static bool exit_requested;
  unsigned max_nesting;};

extern Variable_map_t* vars;
extern int& dollar_question;

inline bool isargvar(const std::string& focus) {
  switch(focus[0]) {
    case '*': case '#': case '0': case '1': case '2': case '3': case '4': 
              case '5': case '6': case '7': case '8': case '9': return true;
    default: return false;}}

