// Copyright (C) 2005-2026 Samuel Newbold

class Variable_map;
class Command_block;

class Argm {
 public:
  Argm(Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i, Rwsh_ostream_p error_i);
  Argm(const Argv& args,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i, Rwsh_ostream_p error_i);
  Argm(const Argv& args, Command_block* argfunction_i,
       Variable_map* parent_map_i);
  Argm(E::Exception_t exception, const Argm& src);
  Argm(const Exception& args);
  Argm(const std::string& first_string, const Argv& subsequent_args,
       Command_block* argfunction_i, Variable_map* parent_map_i,
       Rwsh_istream_p input_i, Rwsh_ostream_p output_i, Rwsh_ostream_p error_i);
  Argm(const Argm& src);
  ~Argm(void);
  Argm& operator=(const Argm& src);
  std::string str(void) const;
  Variable_map* parent_map(void) const {return parent_map_v;};
  Variable_map* nonempty_parent_map(void) const;
  Command_block* argfunction(void) const {return argfunction_v;};
  void set_argfunction(Command_block* val);

  mutable Rwsh_istream_p input;
  mutable Rwsh_ostream_p output, error;

// variables
  void export_env(std::vector<char*>& env) const;

  std::string get_var(const std::string& key) const;
  void global(const std::string& key, const std::string& value) const;
  void local(const std::string& key, const std::string& value) const;
  void local_declare(const std::string& key, Error_list& exceptions) const;
  void set_var(const std::string& key, const std::string& value) const;
  template<class Out>
  Out star_var(const std::string& key, unsigned reference_level, Out res) const;
  void unset_var(const std::string& key) const;
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
  void add_error(const Exception& error);
  void add_error(const Argm& error);
  void replace_error(const Argm& error);
  void reset(void); };
