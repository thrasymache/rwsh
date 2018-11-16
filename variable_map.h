// Copyright (C) 2006-2018 Samuel Newbold

extern const char* WSPACE;
std::string escape(const std::string& src);
std::string word_from_value(const std::string& value);

class Variable_map : private std::map<std::string, std::string> {
  typedef std::map<std::string, std::string> Base;
  std::set<std::string> checked_vars;
  std::set<std::string> local_vars;
  std::set<std::string> undefined_vars;
  std::set<std::string> used_vars;
  Variable_map* parent;

  const Variable_map* parent_with(const std::string& key) const;
  void export_env(std::vector<char*>& env, const Variable_map* descendant);
public:
  typedef Base::iterator iterator;
  typedef Base::const_iterator const_iterator;
  static Variable_map *global_map;
  static int dollar_question;
  static bool exit_requested;
  bool locals_listed;
  bool usage_checked;

  Variable_map(Variable_map* parent);
  void bless_unused_vars();
  ~Variable_map();

  const_iterator begin(void) const {return Base::begin();};
  const_iterator end(void) const {return Base::end();};
  iterator begin(void) {return Base::begin();};
  iterator end(void) {return Base::end();};
  unsigned size(void) const {return Base::size();};

  void add_undefined(const std::string& key) {undefined_vars.insert(key);}
  void append_word_locally(const std::string& key, const std::string& value);
  void append_word_if_exists(const std::string& key, const std::string& value);
  void param_or_append_word(const std::string& key, const std::string& value);
  template <class In, class Out>
      Out copy_to_char_star_star(
              In first, In last, Out res, const Variable_map* descendant);
  bool exists(const std::string& key, bool check);
  void export_env(std::vector<char*>& env);
  const std::string& get(const std::string& key);
  void global(const std::string& key, const std::string& value);
  void param(const std::string& key, const std::string& value);
  void local(const std::string& key, const std::string& value);
  const std::set<std::string>& locals(void) const {return local_vars;};
  void set(const std::string& key, const std::string& value);
  bool simple_exists(const std::string& key) const {return find(key) != end();}
  void unset(const std::string& key);
  bool used_vars_contains(const std::string& key) const {
    return used_vars.find(key) !=  used_vars.end();};
  bool checked_vars_contains(const std::string& key) const {
    return checked_vars.find(key) !=  checked_vars.end();};
  void used_vars_insert(const std::string& key) {
    used_vars.insert(key);}; };

inline bool isargvar(const std::string& focus) {
  switch(focus[0]) {
    case '*': case '#': case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9': return true;
    default: return false;}}

