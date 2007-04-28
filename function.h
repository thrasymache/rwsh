// Copyright (C) 2005, 2006 Samuel Newbold

class Function_t : public Executable_t {
  std::string name_v;

  Function_t(const std::string& name_i) : name_v(name_i), script() {};
  Function_t(const std::string& name_i, const std::vector<Arg_script_t>& src) : 
    name_v(name_i), script(src) {};
 public:
  std::vector<Arg_script_t> script;

  Function_t(const std::string& name, const std::string& src);
  Function_t(const std::string& name_i, const Function_t* src) :
    name_v(name_i), script(src->script) {};
  Function_t* copy_pointer(void) const {
    if (!this) return 0;
    else return new Function_t(name_v, script);};
  int operator() (const Argv_t& src_argv);
  int operator() (const Argv_t& src_argv, Rwsh_stream_t* override_stream);
  Function_t* apply(const Argv_t& argv) const ;
  const std::string& name(void) const {return name_v;};
  std::string str() const; 

  // vector semantics for argument scripts
  typedef std::vector<Arg_script_t>::value_type value_type;
  typedef std::vector<Arg_script_t>::allocator_type allocator_type;
  typedef std::vector<Arg_script_t>::size_type size_type;
  typedef std::vector<Arg_script_t>::difference_type difference_type;
  typedef std::vector<Arg_script_t>::iterator iterator;
  typedef std::vector<Arg_script_t>::const_iterator const_iterator;
  typedef std::vector<Arg_script_t>::reverse_iterator reverse_iterator;
  typedef std::vector<Arg_script_t>::const_reverse_iterator 
      const_reverse_iterator;
  typedef std::vector<Arg_script_t>::reference reference;
  typedef std::vector<Arg_script_t>::const_reference const_reference;
  typedef std::vector<Arg_script_t>::pointer pointer;
  typedef std::vector<Arg_script_t>::const_pointer const_pointer; };

