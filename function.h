// Copyright (C) 2005-2015 Samuel Newbold

class Command_block : public Base_executable, public std::vector<Arg_script> {
 public:
  Command_block() {};
  Command_block(const std::string& src, std::string::size_type& point,
                unsigned max_soon);
  Command_block* copy_pointer(void) const {
    if (!this) return 0;
    else return new Command_block(*this);};
  Command_block* apply(const Argm& argm, unsigned nesting) const;
  int internal_execute(const Argm& src_argm);
  int operator() (const Argm& src_argm);
  void promote_soons(unsigned nesting);
  std::string str() const; };

struct Parameter_group {
  bool required;
  std::vector<std::string> names;
  Parameter_group() : required(true), names() {};
  Parameter_group(bool required_i, const std::string& first_name) :
      required(required_i), names(1, first_name) {}; };

class Function : public Named_executable {
  std::string name_v;
  std::vector<Parameter_group> positional;
  unsigned required_argc;
  std::map<std::string, Parameter_group> flag_options;
  std::set<std::string> parameter_names;
  bool positional_parameters;
  bool all_flags;

  Function(const std::string& name_i) :
      name_v(name_i), positional(), required_argc(0), flag_options(),
      parameter_names(), positional_parameters(true), all_flags(true),
      body() {};
 public:
  Command_block body;

  Function(const std::string& name, const std::string& src,
           std::string::size_type& point, unsigned max_soon);
  Function(const std::string& name, const std::string& src);
  Function(const Function& src) :
    name_v(src.name_v), positional(src.positional),
    required_argc(src.required_argc), flag_options(src.flag_options),
    parameter_names(src.parameter_names),
    positional_parameters(src.positional_parameters), all_flags(src.all_flags),
    body(src.body) {};
  Function(const std::string& name_i, Argm::const_iterator first_parameter,
           Argm::const_iterator parameter_end, bool positional_parameters_i,
           bool all_flags_i, const Command_block& src);
  Function* copy_pointer(void) const {
    if (!this) return 0;
    else return new Function(*this);};
  int operator() (const Argm& src_argm);
  Function* apply(const Argm& argm, unsigned nesting) const;
  const std::string& name(void) const {return name_v;};
  void promote_soons(unsigned nesting);
  std::string str() const;};

struct Unclosed_brace {
  std::string prefix;
  Unclosed_brace(const std::string& prefix_i) : prefix(prefix_i) {}};

