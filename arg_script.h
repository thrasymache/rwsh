// Copyright (C) 2006 Samuel Newbold

enum Arg_type_t {FIXED, VARIABLE, STAR_VAR, SELECTION, SELECT_VAR,
                 SELECT_STAR_VAR};

class Arg_spec_t {
  Arg_type_t type;
  unsigned reference_level;
  std::string text;
 public:
  Arg_spec_t(const std::string& script);
  std::string str(void) const;
  template<class Out> void interpret(const Argv_t& src, Out dest) const; };

struct Arguments_to_argfunction_t : public Argv_t {
  Arguments_to_argfunction_t() : Argv_t(default_stream_p) {};};

class Arg_script_t : private std::vector<Arg_spec_t> {
  typedef std::vector<Arg_spec_t> Base;
  unsigned argfunction_level;
  Function_t* argfunction;

 public:
  Arg_script_t(const Argv_t& src);
  Arg_script_t(const Arg_script_t& src);
  Arg_script_t& operator=(const Arg_script_t& src);
  ~Arg_script_t(void);
  std::string str(void) const;
  Argv_t interpret(const Argv_t& src) const;
  Arg_script_t apply(const Argv_t& src) const;
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
bool is_function_name(const std::string& focus);
