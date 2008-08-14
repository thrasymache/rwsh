// Copyright (C) 2006, 2007 Samuel Newbold

class Arg_script_t {
  std::vector<Arg_spec_t> args;
  Function_t* argfunction;
  unsigned argfunction_level;
  Rwsh_istream_p input;
  Rwsh_ostream_p output, error;

  Arg_script_t(const Rwsh_istream_p& input, const Rwsh_ostream_p& output,
               const Rwsh_ostream_p& error);
  std::string::size_type add_literal(const std::string& src,
                                     std::string::size_type token_start,
                                     unsigned max_soon);
  std::string::size_type parse_token(const std::string& src,
                                     std::string::size_type token_start,
                                     unsigned max_soon);
  std::string::size_type add_token(const std::string& src,
                                   std::string::size_type token_start,
                                   std::string::size_type token_end,
                                   unsigned max_soon);
  std::string::size_type add_function(const std::string& src,
                                      std::string::size_type style_start,
                                      std::string::size_type f_start,
                                      unsigned max_soon);
  std::string::size_type find_close_brace(const std::string& focus,
                                          std::string::size_type i);
 public:
  Arg_script_t(const std::string& src, unsigned max_soon);
  Arg_script_t(const Arg_script_t& src);
  Arg_script_t& operator=(const Arg_script_t& src);
  ~Arg_script_t(void);
  Argv_t argv(void) const;
  void apply(const Argv_t& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_script_t> > res) const;
  std::string str(void) const;
  Argv_t interpret(const Argv_t& src) const;
  void promote_soons(unsigned);
  bool is_argfunction(void) const {return argfunction_level == 1;}; };

bool is_argfunction_name(const std::string& focus);
bool is_binary_name(const std::string& focus);
bool is_builtin_name(const std::string& focus);
bool is_internal_function_name(const std::string& focus);
bool is_function_name(const std::string& focus);
