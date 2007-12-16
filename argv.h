// Copyright (C) 2005, 2006, 2007 Samuel Newbold

class Function_t;
class Rwsh_stream_t;
class Variable_map_t;

class Argv_t : private std::vector<std::string> {
  typedef std::vector<std::string> Base;
  Function_t* argfunction_v;
  Rwsh_stream_t* myout_v;
  static Variable_map_t* var_map;

 public:
  Argv_t(void);
  template <class In> Argv_t(In first, In last, Function_t* argfunction_i, 
                             Rwsh_stream_t* myout_i) :
    Base(first, last), argfunction_v(argfunction_i->copy_pointer()), 
    myout_v(myout_i) {};
  Argv_t(const Argv_t& src);
  ~Argv_t(void);
  Argv_t& operator=(const Argv_t& src);
  std::string str(void) const;
  Function_t* argfunction(void) const {return argfunction_v;};
  Rwsh_stream_t* myout(void) const {return myout_v;};
  void set_myout(Rwsh_stream_t* val);
  void set_argfunction(Function_t* val);

// variables
  void append_to_errno(const std::string& value) const;
  char** export_env(void) const;
  std::string get_var(const std::string& key) const;
  int global_var(const std::string& key, const std::string& value) const;
  unsigned max_nesting(void) const;
  int set_var(const std::string& key, const std::string& value) const;
  template<class Out> 
  Out star_var(const std::string& key, unsigned reference_level, Out res) const;
  int unset_var(const std::string& key) const;
  bool var_exists(const std::string& key) const;

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
  const_reference front(void) const {return Base::front();};
  reference front(void) {return Base::front();};
  const_reference back(void) const {return Base::back();};
  reference back(void) {return Base::back();};
  iterator erase(iterator key) {return Base::erase(key);}
  iterator erase(iterator first, iterator last) {
    return Base::erase(first, last);}
  void clear(void);
  void push_front(const std::string& x) {Base::insert(Base::begin(), x);};
  void pop_front(void) {Base::erase(Base::begin());};
  void push_back(const std::string& x) {Base::push_back(x);};
  size_type size(void) const {return Base::size();};
  std::string& operator[] (int i) {return Base::operator[](i);};
  const std::string& operator[] (int i) const {return Base::operator[](i);}; };

struct Arguments_to_argfunction_t : public Argv_t {
  Arguments_to_argfunction_t(const std::string& argfunction_type);};

struct Bad_argfunction_style_t : public Argv_t {
  Bad_argfunction_style_t(const std::string& argfunction_style);};

struct Double_redirection_t : public Argv_t {
  Double_redirection_t(const std::string& first, const std::string& second);};

struct File_open_failure_t : public Argv_t {
  File_open_failure_t(const std::string& file_name);};

struct Failed_substitution_t : public Argv_t {
  Failed_substitution_t(const std::string& function);};

struct Mismatched_brace_t : public Argv_t {
  Mismatched_brace_t(const std::string& prefix);};

struct Multiple_argfunctions_t : public Argv_t {Multiple_argfunctions_t();};

struct Not_soon_enough_t : public Argv_t {
  Not_soon_enough_t(const std::string& argument);};

struct Undefined_variable_t : public Argv_t {
  Undefined_variable_t(const std::string& variable);};
  
class Old_argv_t {
  char** focus;
  int argc_v;

 public:
  Old_argv_t(const Argv_t& src);
  ~Old_argv_t(void);
  char** argv(void) {return focus;};
  int argc(void) {return argc_v;}; };

