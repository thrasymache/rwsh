// Copyright (C) 2015-2019 Samuel Newbold

enum Flag_type {ALL, SOME};
enum Dash_dash_type {UNSEEN, BRACKET, BARE};

struct Parameter_group {
  typedef std::vector<std::string> Base;
  Base::difference_type elipsis;
  bool has_argfunction;
  Base names;
  bool required;
  Parameter_group() : has_argfunction(false), required(true), elipsis(-2),
                      names() {};
  Parameter_group(bool required_i):  has_argfunction(false),
                      required(required_i), elipsis(-2), names() {};
  Parameter_group(Argm::const_iterator& fp, Argm::const_iterator end,
                  std::set<std::string>& parameter_names);
  void arg_to_param(int& available, int& needed, std::string& missing,
                    Argm::const_iterator& f_arg,
                    const Argm::const_iterator end,
                    const std::string* flag, const std::string& elipsis_var,
                    enum Dash_dash_type dash_dash, bool is_reassign,
                    Variable_map& locals, Error_list& exceptions) const;
  void add_undefined_params(Variable_map& locals, bool is_reassign) const;
  std::string str() const;
  void bless_unused_vars(Variable_map* vars) const;
  void p_elipsis(Variable_map& locals, Argm::const_iterator& f_arg,
                 int& available, const std::string& name,
                 const std::string* flag, int needed,
                 enum Dash_dash_type dash_dash, bool is_reassign,
                 Error_list& exceptions) const;
  char unused_flag_var_check(Variable_map* vars, Error_list& exceptions) const;
  void unused_pos_var_check(Variable_map* vars,
                            Error_list& exceptions) const; };

class Prototype {
  bool bare_dash_dash;
  int dash_dash_position;
  std::string elipsis_var;
  std::map<std::string, Parameter_group> flag_options;
  Flag_type flags;
  std::set<std::string> parameter_names;
  std::vector<Parameter_group> positional;
  unsigned required_argc;

  void arg_to_param_internal(const Argv& invoking_argm, bool is_reassign,
                    Variable_map& locals, Error_list& exceptions) const;

 public:
  bool exclude_argfunction;
  bool required_argfunction;
  Prototype(void);
  Prototype(const Argv& parameters);
  void reassign(const Argv& invoking_argm, Variable_map& locals,
                Error_list& exceptions) const;
  void arg_to_param(const Argv& invoking_argm, Variable_map& locals,
                    Error_list& exceptions) const;
  void bad_args(std::string& missing, Variable_map& locals,
                Argm::const_iterator f_arg, Argm::const_iterator end,
                Error_list& exceptions) const;
  std::string str() const;
  void unused_var_check(Variable_map* vars, Error_list& exceptions) const; };
