// Copyright (C) 2005-2015 Samuel Newbold

class Function : public Executable {
  std::string name_v;
  std::vector<std::string> parameters;
  bool positional_parameters;

  Function(const std::string& name_i) : name_v(name_i), parameters(),
                                       positional_parameters(true), script() {};
  void internal_constructor(const std::string& src,
                            std::string::size_type& point, unsigned max_soon);
 public:
  std::vector<Arg_script> script;

  Function(const std::string& name, const std::string& src,
           std::string::size_type& point, unsigned max_soon);
  Function(const std::string& name, const std::string& src);
  Function(const std::string& name_i, Argm::const_iterator first_parameter,
           Argm::const_iterator parameter_end, bool positional_parameters_i,
           const std::vector<Arg_script>& src) : 
    name_v(name_i), parameters(first_parameter, parameter_end),
    positional_parameters(positional_parameters_i), script(src) {};
  Function* copy_pointer(void) const {
    if (!this) return 0;
    else return new Function(name_v, parameters.begin(), parameters.end(),
                             positional_parameters, script);};
  int operator() (const Argm& src_argm);
  Function* apply(const Argm& argm, unsigned nesting) const ;
  const std::string& name(void) const {return name_v;};
  Function* promote_soons(unsigned nesting) const;
  std::string str() const; 

  // vector semantics for argument scripts
  typedef std::vector<Arg_script>::value_type value_type;
  typedef std::vector<Arg_script>::allocator_type allocator_type;
  typedef std::vector<Arg_script>::size_type size_type;
  typedef std::vector<Arg_script>::difference_type difference_type;
  typedef std::vector<Arg_script>::iterator iterator;
  typedef std::vector<Arg_script>::const_iterator const_iterator;
  typedef std::vector<Arg_script>::reverse_iterator reverse_iterator;
  typedef std::vector<Arg_script>::const_reverse_iterator 
      const_reverse_iterator;
  typedef std::vector<Arg_script>::reference reference;
  typedef std::vector<Arg_script>::const_reference const_reference;
  typedef std::vector<Arg_script>::pointer pointer;
  typedef std::vector<Arg_script>::const_pointer const_pointer; };

struct Unclosed_brace {
  std::string prefix;
  Unclosed_brace(const std::string& prefix_i) : prefix(prefix_i) {}};

