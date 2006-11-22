// Copyright (C) 2005, 2006 Samuel Newbold

class Function_t;
class Variable_map_t;

class Argv_t : private std::vector<std::string> {
  typedef std::vector<std::string> Base;
  Function_t* argfunction_v;
  void add_tokens(const std::string& src);
  static Variable_map_t* var_map;

 public:
  Argv_t(void) : argfunction_v(0) {};
  Argv_t(const std::string& src);
  template <class In> Argv_t(In first, In last, Function_t* argfunction_i) :
    Base(first, last), argfunction_v(argfunction_i->copy_pointer()) {};
  Argv_t(const Argv_t& src);
  ~Argv_t(void);
  Argv_t& operator=(const Argv_t& src);
  std::string str(void) const;
  Function_t* argfunction(void) const {return argfunction_v;};
  void set_argfunction(Function_t* val) {argfunction_v = val;};
  std::string get_var(const std::string& key) const;
  void set_var(const std::string& key, const std::string& value) const;
  unsigned max_nesting(void) const;
  char** export_env(void) const;
  template<class Out> Out star_var(const std::string& key, Out res) const;

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
  const_reference back(void) const {return Base::back();};
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

class Old_argv_t {
  char** focus;
  int argc_v;

 public:
  Old_argv_t(const Argv_t& src);
  ~Old_argv_t(void);
  char** argv(void) {return focus;};
  int argc(void) {return argc_v;}; };

