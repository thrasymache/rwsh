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

struct Arguments_to_argfunction_t : public Argv_t {
  Arguments_to_argfunction_t(const std::string& argfunction_type);};

struct Bad_argfunction_style_t : public Argv_t {
  Bad_argfunction_style_t(const std::string& argfunction_style);};

struct Failed_substitution_t : public Argv_t {
  Failed_substitution_t(const std::string& function);};

struct Mismatched_brace_t : public Argv_t {
  Mismatched_brace_t(const std::string& prefix);};

struct Multiple_argfunctions_t : public Argv_t {Multiple_argfunctions_t();};

struct Not_soon_enough_t : public Argv_t {
  Not_soon_enough_t(const std::string& argument);};

struct Undefined_variable_t : public Argv_t {
  Undefined_variable_t(const std::string& variable);};
  
class Arg_script_t : private std::vector<Arg_spec_t> {
  typedef std::vector<Arg_spec_t> Base;
  Function_t* argfunction;
  unsigned argfunction_level;
  Rwsh_stream_t* myout;

  void add_tokens(const std::string& s);

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
