// Copyright (C) 2015-2026 Samuel Newbold

class Error_list;
class Prototype;
class Variable_map;

enum Flag_type {ALL, SOME};
enum Dash_dash_type {UNSEEN, BRACKET, BARE};

class Parsing_state {
  const Argv::const_iterator end;
 public:
  int available;
  Argv::const_iterator cur_arg;
  int needed;
  int pos_c;
  enum Dash_dash_type dash_dash;
  Variable_map& locals;
  std::string missing;
  Error_list& exceptions;
  Parsing_state(const Argv& argv, int needed, int pos_c,
                enum Dash_dash_type dash_dash, Variable_map& locals,
                Error_list& exceptions);
  void append_cur_arg(const std::string& key, bool is_reinterpret);
  bool is_flag_arg(void) const {
    return (*cur_arg)[0] == '-' && dash_dash != BARE && cur_arg->length() > 1;}
  bool cur_is_end(void) const {return cur_arg == end;}
  bool excess_or_missing_args(void) const {
    return cur_arg != end || needed || missing.length();}
  void add_error(Exception focus);
  void set_get_word_from_value(const std::string& key);
};

struct Parameter_group {
  typedef std::vector<std::string> Base;
  Base::difference_type elipsis;
  bool has_argfunction;
  Base names;
  const Prototype& parent;
  bool required;

  Parameter_group(Argv::const_iterator& fp, Argv::const_iterator end,
                  std::set<std::string>& parameter_names,
                  const Prototype& parent);
  void arg_to_param(Parsing_state& state, const std::string* flag) const;
  void add_undefined_params(Variable_map& locals) const;
  std::string str() const;
  void bless_unused_vars(Variable_map* vars) const;
  void p_elipsis(Parsing_state& state, const std::string* flag) const;
  char unused_flag_var_check(Variable_map* vars, Error_list& exceptions) const;
  void unused_pos_var_check(Variable_map* vars, Error_list& exceptions) const;};

class Prototype {
  bool bare_dash_dash;
  int dash_dash_position;
  std::map<std::string, Parameter_group> flag_options;
  Flag_type flags;
  std::set<std::string> parameter_names;
  std::vector<Parameter_group> positional;
  unsigned required_argc;
  int pos_argc;

 public:
  std::string elipsis_var;
  bool exclude_argfunction;
  bool required_argfunction;
  bool is_reinterpret;
  bool is_extra_round;
  Prototype(void);
  Prototype(const Argv& parameters, bool is_reinterpret, bool is_extra_round);
  void arg_to_param(const Argv& invoking_argm, Variable_map& locals,
                    Error_list& exceptions) const;
  void bad_args(Parsing_state& state) const;
  std::string str() const;
  void unused_var_check(Variable_map* vars, Error_list& exceptions) const; };
