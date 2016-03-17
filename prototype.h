// Copyright (C) 2015-2016 Samuel Newbold

enum Flag_type {ALL, SOME, IGNORANT};

struct Parameter_group {
  bool required;
  std::vector<std::string>::difference_type elipsis;
  std::vector<std::string> names;
  Parameter_group() : required(true), elipsis(-2), names() {};
  Parameter_group(bool required_i): required(required_i),elipsis(-2),names() {};
  std::string str() const; };

class Prototype {
  bool non_prototype;
  bool explicit_dash_dash;
  Flag_type flags;
  unsigned required_argc;
  std::map<std::string, Parameter_group> flag_options;
  std::set<std::string> parameter_names;
  std::vector<Parameter_group> positional;
 public:
  Prototype(bool non_prototype);
  Prototype(Argm::const_iterator first_parameter,
            Argm::const_iterator parameter_end,
            bool non_prototype_i, Flag_type flags_i);
  Variable_map arg_to_param(const Argm& invoking_argm);
  std::string str(const std::string &function_name) const;};
