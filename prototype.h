// Copyright (C) 2015-2016 Samuel Newbold

enum Flag_type {ALL, SOME};
enum Dash_dash_type {UNSEEN, BRACKET, BARE};

std::string escape(const std::string& src);

struct Parameter_group {
  bool required;
  std::vector<std::string>::difference_type elipsis;
  std::vector<std::string> names;
  Parameter_group() : required(true), elipsis(-2), names() {};
  Parameter_group(bool required_i): required(required_i),elipsis(-2),names() {};
  Parameter_group(Argm::const_iterator& fp, Argm::const_iterator end,
                  std::set<std::string>& parameter_names);
  void arg_to_param(Variable_map& locals, int& available, int& needed,
                    std::string& missing,
                    Argm::const_iterator& f_arg,
                    const Argm::const_iterator end,
                    const std::string* flag,
                    const std::string& elipsis_var,
                    enum Dash_dash_type dash_dash) const;
  std::string str() const; };

class Prototype {
  bool bare_dash_dash;
  int dash_dash_position;
  Flag_type flags;
  unsigned required_argc;
  std::string elipsis_var;
  std::map<std::string, Parameter_group> flag_options;
  std::set<std::string> parameter_names;
  std::vector<Parameter_group> positional;
 public:
  bool non_prototype;
  Prototype(bool non_prototype);
  Prototype(Argm::const_iterator first_parameter,
            Argm::const_iterator parameter_end,
            bool non_prototype_i, Flag_type flags_i);
  Variable_map arg_to_param(const Argm& invoking_argm) const;
  Argm bad_args(std::string& missing, const Variable_map& locals,
                Argm::const_iterator f_arg, Argm::const_iterator end,
                std::vector<Parameter_group>::const_iterator param) const;
  std::string str() const;};