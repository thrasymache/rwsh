// Copyright (C) 2006, 2007 Samuel Newbold

enum Arg_type_t {FIXED, REFERENCE, SOON, STAR_REF, SELECTION, SELECT_VAR, 
                 SELECT_STAR_VAR, SUBSTITUTION};

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
  Arg_spec_t(const Arg_spec_t& src);
  ~Arg_spec_t();
  void apply(const Argv_t& src, unsigned nesting);
  template<class Out> void interpret(const Argv_t& src, Out dest) const;
  void promote_soons(unsigned nesting);
  std::string str(void) const; };

class Arg_script_t : private std::vector<Arg_spec_t> {
  typedef std::vector<Arg_spec_t> Base;
  Function_t* argfunction;
  unsigned argfunction_level;
  Rwsh_stream_p myout;

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
  bool is_argfunction(void) const {return argfunction_level == 1;};

// vector semantics
  typedef Base::value_type value_type;
  typedef Base::allocator_type allocator_type;
  typedef Base::size_type size_type;
  typedef Base::difference_type difference_type;
  typedef Base::iterator iterator;
  typedef Base::const_iterator const_iterator;
  typedef Base::reverse_iterator reverse_iterator;
  typedef Base::const_reverse_iterator const_reverse_iterator;
  typedef Base::reference reference;
  typedef Base::pointer pointer;
  typedef Base::const_pointer const_pointer;
  typedef Base::reference reference;
  typedef Base::const_reference const_reference;

  iterator begin(void) {return Base::begin();};
  const_iterator begin(void) const {return Base::begin();};
  iterator end(void) {return Base::end();};
  const_iterator end(void) const {return Base::end();};
  void clear(void);
  void push_back(const Arg_spec_t& x) {Base::push_back(x);};
  size_type size(void) const {return Base::size();};
  Arg_spec_t& operator[] (int i) {return Base::operator[](i);};
  const Arg_spec_t& operator[] (int i) const {return Base::operator[](i);}; };

bool is_argfunction_name(const std::string& focus);
bool is_binary_name(const std::string& focus);
bool is_builtin_name(const std::string& focus);
bool is_internal_function_name(const std::string& focus);
bool is_function_name(const std::string& focus);
