// Copyright (C) 2005, 2006, 2007 Samuel Newbold

class Function;
class Variable_map;

class Argv : private std::vector<std::string> {
  typedef std::vector<std::string> Base;
  Function* argfunction_v;
  static Variable_map* var_map;

 public:
  Argv(void);
  template <class String_it> 
  Argv(String_it first_string, String_it last_string,
         Function* argfunction_i, 
         Rwsh_istream_p input_i, Rwsh_ostream_p output_i,
         Rwsh_ostream_p error_i);
  Argv(const Argv& src);
  ~Argv(void);
  Argv& operator=(const Argv& src);
  std::string str(void) const;
  Function* argfunction(void) const {return argfunction_v;};
  void set_argfunction(Function* val);

  mutable Rwsh_istream_p input;
  mutable Rwsh_ostream_p output, error;

  enum Sig_type {
    Argument_count,
    Arguments_for_argfunction,
    Bad_argfunction_style,
    Bad_if_nest, 
    Binary_not_found,
    Divide_by_zero, 
    Double_redirection,
    Else_without_if,
    Excess_argfunction,
    Excessive_nesting,
    Executable_not_found,
    Failed_substitution,
    File_open_failure,
    If_before_else,
    Input_range,
    Invalid_word_selection,
    Line_continuation,
    Mismatched_brace,
    Mismatched_parenthesis,
    Missing_argfunction,
    Multiple_argfunctions,
    Not_a_number,
    Not_executable,
    Not_soon_enough,
    Result_range,
    Selection_not_found,
    Sighup,
    Sigint,
    Sigquit,
    Sigpipe,
    Sigterm,
    Sigtstp,
    Sigcont,
    Sigchld,
    Sigusr1,
    Sigusr2,
    Sigunknown,
    Undefined_variable,
    Unreadable_dir,
    Signal_count};

  static std::string signal_names[];

// variables
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
  void push_front(const std::string& x) {Base::insert(Base::begin(), x);};
  void pop_front(void) {Base::erase(Base::begin());};
  void push_back(const std::string& x) {Base::push_back(x);};
  size_type size(void) const {return Base::size();};
  std::string& operator[] (int i) {return Base::operator[](i);};
  const std::string& operator[] (int i) const {return Base::operator[](i);}; };

struct Signal_argv : public Argv {
  Signal_argv(Sig_type signal);
  Signal_argv(Sig_type signal, const std::string& value);
  Signal_argv(Sig_type signal, const std::string& x, const std::string& y);
  Signal_argv(Sig_type signal, int x, int y);};

class Old_argv {
  char** focus;
  int argc_v;

 public:
  Old_argv(const Argv& src);
  ~Old_argv(void);
  char** argv(void) {return focus;};
  int argc(void) {return argc_v;}; };

