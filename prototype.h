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
                    enum Dash_dash_type dash_dash,
                    Variable_map& locals, Error_list& exceptions) const;
  void add_undefined_params(Variable_map& locals) const;
  std::string str() const;
  void bless_unused_vars(Variable_map* vars) const;
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
 public:
  bool non_prototype;
  bool exclude_argfunction;
  bool required_argfunction;
  Prototype(bool non_prototype);
  Prototype(const Argv& parameters);
  void arg_to_param(const Argm& invoking_argm, Variable_map& locals,
                    Error_list& exceptions) const;
  void bad_args(std::string& missing, const Variable_map& locals,
                Argm::const_iterator f_arg, Argm::const_iterator end,
                std::vector<Parameter_group>::const_iterator param,
                Error_list& exceptions) const;
  std::string str() const;
  void unused_var_check(Variable_map* vars, Error_list& exceptions) const; };
