// Copyright (C) 2005-2015 Samuel Newbold

class Command_block : public Base_executable, public std::vector<Arg_script> {
  typedef std::vector<Arg_script> Base;
 public:
  std::string trailing;

  Command_block() {};
  Command_block(const Command_block& src) : Base(src), trailing(src.trailing) {
    };
  Command_block& operator=(const Command_block& src) {
    Base::clear();
    std::copy(src.begin(), src.end(), std::back_inserter(*this));
    trailing = src.trailing;};
  Command_block(const std::string& src, std::string::size_type& point,
                unsigned max_soon);
  Command_block* copy_pointer(void) const {
    if (!this) return 0;
    else {
      Command_block* result = new Command_block(*this);
      return result;}};
  Command_block* apply(const Argm& argm, unsigned nesting) const;
  int internal_execute(const Argm& src_argm);
  int operator() (const Argm& src_argm);
  void promote_soons(unsigned nesting);
  std::string str() const; };

struct Parameter_group {
  bool required;
  std::vector<std::string>::difference_type elipsis;
  std::vector<std::string> names;
  Parameter_group() : required(true), elipsis(-2), names() {};
  Parameter_group(bool required_i): required(required_i),elipsis(-2),names() {};
  std::string str() const; };

enum Flag_type {ALL, SOME, IGNORANT};

class Function : public Named_executable {
  std::string name_v;
  std::vector<Parameter_group> positional;
  unsigned required_argc;
  std::map<std::string, Parameter_group> flag_options;
  std::set<std::string> parameter_names;
  bool non_prototype;
  Flag_type flags;
  bool explicit_dash_dash;

  Function(const std::string& name_i) :
      name_v(name_i), positional(), required_argc(0), flag_options(),
      parameter_names(), non_prototype(true), flags(ALL),
      body() {};
  void check_for_duplicates(const std::string& name);
 public:
  Command_block body;

  Function(const std::string& name, const std::string& src,
           std::string::size_type& point, unsigned max_soon);
  Function(const std::string& name, const std::string& src);
  Function(const Function& src) :
    name_v(src.name_v), positional(src.positional),
    required_argc(src.required_argc), flag_options(src.flag_options),
    parameter_names(src.parameter_names),
    non_prototype(src.non_prototype), flags(src.flags),
    explicit_dash_dash(src.explicit_dash_dash), body(src.body) {};
  Function(const std::string& name_i, Argm::const_iterator first_parameter,
           Argm::const_iterator parameter_end, bool non_prototype_i,
           Flag_type flags_i, const Command_block& src);
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

