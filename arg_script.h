// Copyright (C) 2006, 2007 Samuel Newbold

enum Arg_type_t {FIXED, REFERENCE, SOON, STAR_REF, STAR_SOON, SELECTION,
                 SELECT_VAR, SELECT_STAR_VAR, SUBSTITUTION};

class Arg_spec_t {
  Arg_type_t type;
  unsigned soon_level;
  unsigned ref_level;
  Function_t* substitution;
  std::string text;

  void add_function_spec(const std::string& src);
 public:
  Arg_spec_t(const std::string& script, unsigned max_soon);
  Arg_spec_t(const std::string& style, const std::string& function, 
             unsigned max_soon);
  Arg_spec_t(Arg_type_t type, unsigned soon_level, unsigned ref_level,
             Function_t* substitution, std::string text);
  Arg_spec_t(const Arg_spec_t& src);
  ~Arg_spec_t();
  void apply(const Argv_t& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_spec_t> > res) const;
  template<class Out> void interpret(const Argv_t& src, Out dest) const;
  void promote_soons(unsigned nesting);
  std::string str(void) const; };

class Arg_script_t {
  std::vector<Arg_spec_t> args;
  Function_t* argfunction;
  unsigned argfunction_level;
  Rwsh_istream_p input;
  Rwsh_ostream_p output, error;

  Arg_script_t(const Rwsh_istream_p& input, const Rwsh_ostream_p& output,
               const Rwsh_ostream_p& error);
  void add_token(const std::string& src, unsigned max_soon);
  void add_function(const std::string& style, const std::string& f_str,
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
