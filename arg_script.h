// Copyright (C) 2006-2015 Samuel Newbold

class Function;

class Arg_script {
  std::vector<Arg_spec> args;
  Function* argfunction;
  unsigned argfunction_level;
  Rwsh_istream_p input;
  Rwsh_ostream_p output, error;
  std::string indent;
  char terminator;

  Arg_script(const Rwsh_istream_p& input, const Rwsh_ostream_p& output,
             const Rwsh_ostream_p& error, const std::string& indent,
             char terminator);
 public:
  static const char* TOKEN_SEPARATORS;

  Arg_script(const std::string& src, unsigned max_soon);
  Arg_script(const std::string& src, std::string::size_type& point,
               unsigned max_soon);
  Arg_script(const Arg_script& src);
  Arg_script& operator=(const Arg_script& src);
  ~Arg_script(void);
  std::string::size_type constructor(const std::string& src,
                              std::string::size_type point, unsigned max_soon);
  Argm argm(void) const;
  void add_token(const std::string& src, unsigned max_soon);
  std::string::size_type add_function(const std::string& src,
                                      std::string::size_type style_start,
                                      std::string::size_type f_start,
                                      unsigned max_soon);
  void apply(const Argm& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_script> > res) const;
  std::string str(void) const;
  Argm interpret(const Argm& src) const;
  void promote_soons(unsigned);
  bool is_argfunction(void) const {return argfunction_level == 1;}; };

struct Unclosed_parenthesis {
  std::string prefix;
  Unclosed_parenthesis(const std::string& prefix_i) : prefix(prefix_i) {}};

bool is_argfunction_name(const std::string& focus);
bool is_binary_name(const std::string& focus);
bool is_builtin_name(const std::string& focus);
bool is_internal_function_name(const std::string& focus);
bool is_function_name(const std::string& focus);
