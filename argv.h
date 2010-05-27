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

struct Argument_count : public Argv {
  Argument_count(unsigned given, unsigned expected);};

struct Missing_argfunction : public Argv {Missing_argfunction();};

struct Arguments_to_argfunction : public Argv {
  Arguments_to_argfunction(const std::string& argfunction_type);};

struct Bad_argfunction_style : public Argv {
  Bad_argfunction_style(const std::string& argfunction_style);};

struct Bad_if_nest : public Argv {Bad_if_nest();};

struct Divide_by_zero : public Argv {
  Divide_by_zero(const std::string& value);};

struct Double_redirection : public Argv {
  Double_redirection(const std::string& first, const std::string& second);};

struct Else_without_if : public Argv {Else_without_if();};

struct Excess_argfunction : public Argv {Excess_argfunction();};

struct Failed_substitution : public Argv {
  Failed_substitution(const std::string& function);};

struct File_open_failure : public Argv {
  File_open_failure(const std::string& file_name);};

struct If_before_else : public Argv {If_before_else();};

struct Invalid_word_selection : public Argv {
  Invalid_word_selection(const std::string& selection);};

struct Input_range : public Argv {Input_range(const std::string& value);};

struct Line_continuation : public Argv {Line_continuation();};

struct Mismatched_brace : public Argv {
  Mismatched_brace(const std::string& prefix);};

struct Mismatched_parenthesis : public Argv {
  Mismatched_parenthesis(const std::string& prefix);};

struct Multiple_argfunctions : public Argv {Multiple_argfunctions();};

struct Not_a_number : public Argv {
  Not_a_number(const std::string& value);};

struct Not_soon_enough : public Argv {
  Not_soon_enough(const std::string& argument);};

struct Not_executable : public Argv {
  Not_executable(const std::string& file_name);};

struct Result_range : public Argv {
  Result_range(const std::string& lhs, const std::string& rhs);};

struct Unclosed_brace : public Argv {
  Unclosed_brace(const std::string& prefix);};

struct Unclosed_parenthesis : public Argv {
  Unclosed_parenthesis(const std::string& prefix);};

struct Undefined_variable : public Argv {
  Undefined_variable(const std::string& variable);};
  
class Old_argv {
  char** focus;
  int argc_v;

 public:
  Old_argv(const Argv& src);
  ~Old_argv(void);
  char** argv(void) {return focus;};
  int argc(void) {return argc_v;}; };

