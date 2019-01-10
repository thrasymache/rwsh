// Copyright (C) 2006-2018 Samuel Newbold

class Command_block;

class Arg_script {
  std::vector<Arg_spec> args;
  Command_block* argfunction;
  unsigned argfunction_level;
  Rwsh_istream_p input;
  Rwsh_ostream_p output, error;
  std::string indent;
  char terminator;

  Arg_script(const Rwsh_istream_p& input, const Rwsh_ostream_p& output,
             const Rwsh_ostream_p& error, const std::string& indent,
             char terminator);
  std::string::size_type add_function(const std::string& src,
         std::string::size_type style_start, std::string::size_type f_start,
         unsigned max_soon, Error_list& errors);
  std::string::size_type add_quote(const std::string& src,
         std::string::size_type point, unsigned max_soon, Error_list& errors);
  void add_token(const std::string& src, unsigned max_soon, Error_list& errors);
  std::string::size_type parse_token(const std::string& src,
         std::string::size_type token_start, unsigned max_soon,
         Error_list& errors);
 public:

  Arg_script(const std::string& src, unsigned max_soon, Error_list& errors);
  Arg_script(const std::string& src, std::string::size_type& point,
               unsigned max_soon, Error_list& errors);
  Arg_script(const Arg_script& src);
  Arg_script& operator=(const Arg_script& src);
  ~Arg_script(void);
  std::string::size_type constructor(const std::string& src,
                              std::string::size_type point, unsigned max_soon,
                              Error_list& errors);
  Argm argm(void) const;
  void apply(const Argm& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_script> > res,
             Error_list& current_exceptions) const;
  std::string str(void) const;
  Argm interpret(const Argm& src, Error_list& exceptions) const;
  void promote_soons(unsigned);
  bool is_argfunction(void) const {return argfunction_level == 1;}; };

bool is_argfunction_name(const std::string& focus);
