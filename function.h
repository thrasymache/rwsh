// Copyright (C) 2005, 2006 Samuel Newbold

class Function_t : public Executable_t {
  std::string name_v;
  std::vector<Arg_script_t> script;
  static int global_nesting;
  static bool excessive_nesting;
  static bool in_excessive_nesting_handler;
  static Argv_t call_stack;

  Function_t(const std::string& name_i) : name_v(name_i) {};
  Function_t(const std::string& name_i, const std::vector<Arg_script_t>& src) : 
    name_v(name_i), script(src) {};
  void excessive_nesting_handler(const Argv_t& src_argv);
 public:
  Function_t(const std::string& name, const std::string& src);
  Function_t(const std::string& name_i, const Function_t* src) :
    name_v(name_i), script(src->script) {};
  Function_t* copy_pointer(void) const {
    if (!this) return 0;
    else return new Function_t(name_v, script);};
  int operator() (const Argv_t& src_argv);
  Function_t* interpret(const Argv_t& argv) const ;
  std::string name(void) const {return name_v;};
  std::string str() const; };

