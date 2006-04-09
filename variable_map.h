// Copyright (C) 2005, 2006 Samuel Newbold

const std::string& get_var(const std::string& key);
void set_var(const std::string& key, const std::string& val);
char** export_env(void);
extern int dollar_question;
extern unsigned max_nesting;
extern bool exit_requested;

inline bool isargvar(const std::string& focus) {
  switch(focus[0]) {
    case '*': case '#': case '0': case '1': case '2': case '3': case '4': 
              case '5': case '6': case '7': case '8': case '9': return true;
    default: return false;}}

