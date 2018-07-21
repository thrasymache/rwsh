// Copyright (C) 2005-2018 Samuel Newbold

class Command_block;

class Argm {
 public:
  typedef std::vector<std::string> Argv;
  Argm(Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i, Rwsh_ostream_p error_i);
  Argm(const Argv& args,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i, Rwsh_ostream_p error_i);
  Argm(const Argv& args, Command_block* argfunction_i,
       Variable_map* parent_map_i);
  Argm(const std::string& first_string, const Argv& subsequent_args,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i, Rwsh_ostream_p error_i);
  Argm(const Argm& src);
  ~Argm(void);
  Argm& operator=(const Argm& src);
  std::string str(void) const;
  Variable_map* parent_map(void) const {return parent_map_v;};
  Command_block* argfunction(void) const {return argfunction_v;};
  void set_argfunction(Command_block* val);

  mutable Rwsh_istream_p input;
  mutable Rwsh_ostream_p output, error;

  enum Exception_t {
    No_exception,
    Ambiguous_prototype_dash_dash,
    Arguments_for_argfunction,
    Bad_argc,
    Bad_argfunction_style,
    Bad_args,
    Bad_if_nest,
    Binary_not_found,
    Dash_dash_argument,
    Dash_star_argument,
    Divide_by_zero,
    Double_redirection,
    Duplicate_parameter,
    Elipsis_first_arg,
    Elipsis_out_of_option_group,
    Else_without_if,
    Excess_argfunction,
    Excessive_exceptions_collected,
    Excessive_exceptions_in_catch,
    Excessive_nesting,
    Executable_not_found,
    Failed_substitution,
    File_open_failure,
    Flag_in_elipsis,
    If_before_else,
    Illegal_variable_name,
    Input_range,
    Internal_error,
    Invalid_word_selection,
    // Line_continuation,
    Mismatched_brace,
    Mismatched_bracket,
    Mismatched_parenthesis,
    Missing_argfunction,
    Multiple_argfunctions,
    Not_a_number,
    Not_executable,
    Not_soon_enough,
    Raw_command,
    Return_code,
    Result_range,
    Post_elipsis_option,
    Post_dash_dash_flag,
    Prompt,
    Selection_not_found,
    Shutdown,
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
    Tardy_flag,
    Unchecked_variable,
    Undefined_variable,
    Unreadable_dir,
    Unrecognized_flag,
    Unused_variable,
    Version_incompatible,
    Exception_count};

  static std::string exception_names[Exception_count];

// variables
  char** export_env(void) const;
  std::string get_var(const std::string& key) const;
  int global(const std::string& key, const std::string& value) const;
  int local(const std::string& key, const std::string& value) const;
  void locals_listed(void) const;
  void set_var(const std::string& key, const std::string& value) const;
  template<class Out>
  Out star_var(const std::string& key, unsigned reference_level, Out res) const;
  int unset_var(const std::string& key) const;
  bool var_exists(const std::string& key) const;

// map semantics
  typedef Argv::size_type size_type;
  typedef Argv::value_type value_type;
  typedef Argv::iterator iterator;
  typedef Argv::reference reference;
  typedef Argv::const_iterator const_iterator;
  typedef Argv::const_reference const_reference;

// vector semantics: to the extent possible this structure should be thought
// of as a map, but it is also a representation of a command line, and so
// there is an assigned order to its members, and some operations should only
// need or want to know that much
  const_iterator begin(void) const {return argv_v.begin();};
  const_iterator end(void) const {return argv_v.end();};
  const_reference back(void) const {return argv_v.back();};
  void push_back(const std::string& x) {argv_v.push_back(x);};
  void pop_back(void) {argv_v.pop_back();};
  reference operator[] (int i) {return argv_v[i];};
  const_reference operator[] (int i) const {return argv_v[i];};

// special vector semantics
  const Argv& argv(void) const {return argv_v;};
  unsigned argc(void) const {return argv_v.size();};
  Argv subrange(unsigned start) const {
    return Argv(begin()+start, end());};
  Argv subrange(unsigned start, unsigned before_end) const {
    return Argv(begin()+start, end()-before_end);};

 private:
  Argv argv_v;
  Command_block* argfunction_v;
  Variable_map* parent_map_v; };

struct Error_list : public std::list<Argm> {
  void add_error(const Argm& error); };

struct Exception : public Argm {
  Argm::Exception_t exception;
  Exception(Exception_t exception);
  Exception(Exception_t exception, const std::string& value);
  Exception(Exception_t exception, const std::string& value, int errno_v);
  Exception(Exception_t exception, const std::string& x, const std::string& y);
  Exception(Exception_t exception, const std::string& w, const std::string& x,
              const std::string& y, const std::string& z);
  Exception(Exception_t exception, int x);
  Exception(Exception_t exception, int x, int y);
  Exception(Exception_t exception, int x, int y, int z);
  Exception(Exception_t exception, const Argm& src);};

struct Unclosed_brace : public Exception {
  Unclosed_brace(const std::string& prefix) :
    Exception(Argm::Mismatched_brace, prefix) {}};

struct Unclosed_parenthesis : public Exception {
  Unclosed_parenthesis(const std::string& prefix_i) :
    Exception(Argm::Mismatched_parenthesis, prefix_i) {}};

class Old_argv {
  char** focus;
  int argc_v;

 public:
  Old_argv(const Argm& src);
  ~Old_argv(void);
  char** argv(void) {return focus;};
  int argc(void) {return argc_v;}; };

