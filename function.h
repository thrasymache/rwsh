// Copyright (C) 2005-2019 Samuel Newbold

class Builtin : public Named_executable {
  void (*implementation)(const Argm& argm, Error_list& exceptions);
  std::string name_v;
  Prototype prototype;
 public:
  Builtin(const std::string& name_i,
          void (*impl)(const Argm& argm, Error_list& exceptions),
          const Argv& prototype_i);
  virtual void execute(const Argm& argm, Error_list& exceptions) const;
  virtual const std::string& name(void) const {return name_v;};
  virtual std::string str() const; };

class Command_block : public Base_executable, public std::vector<Arg_script> {
  typedef std::vector<Arg_script> Base;
 public:
  std::string trailing;

  Command_block() {};
  Command_block(const Command_block& src) : Base(src), trailing(src.trailing) {
    };
  Command_block& operator=(const Command_block& src) {
    Base::clear();
    std::copy(src.begin(), src.end(), std::back_inserter(*this));
    trailing = src.trailing;};
  Command_block(const std::string& src, std::string::size_type& point,
                unsigned max_soon, Error_list& errors);
  Command_block* copy_pointer(void) const {
    if (!this) return 0;
    else {
      Command_block* result = new Command_block(*this);
      return result;}};
  Command_block* apply(const Argm& argm, unsigned nesting,
                       Error_list& exceptions) const;

  void collect_errors_core(const Argm& src_argm,
                          const std::vector<std::string>& exceptional,
                          bool logic, Error_list& exceptions);
  virtual void execute(const Argm& argm, Error_list& exceptions) const;
  void promote_soons(unsigned nesting);
  void prototype_execute(const Argm& argm, const Prototype& prototype,
                        Error_list& exceptions) const;
  std::string str() const; };

class Function : public Named_executable {
  std::string name_v;
  Prototype prototype;

  Function(const std::string& name_i) :
      name_v(name_i), prototype(true), body() {};
  void check_for_duplicates(const std::string& name);
 public:
  Command_block body;

  Function(const std::string& name, const Argv& parameters,
           const std::string& src, Error_list& errors);
  Function(const std::string& name_i, bool non_prototype_i,
           const Command_block& src);
  Function(const std::string& name_i, const Argv& parameters,
           const Command_block& src);
  Function(const Function& src) :
    name_v(src.name_v), prototype(src.prototype), body(src.body) {}
  void arg_to_param(const Argm& invoking_argm, Variable_map& locals,
                    Error_list& exceptions) const {
    return prototype.arg_to_param(invoking_argm, locals, exceptions);}
  Function* copy_pointer(void) const {
    if (!this) return 0;
    else return new Function(*this);};
  virtual void execute(const Argm& argm, Error_list& exceptions) const;
  const std::string& name(void) const {return name_v;};
  void promote_soons(unsigned nesting);
  std::string str() const;
  void unused_var_check(Variable_map* vars, Error_list& exceptions) const {
    prototype.unused_var_check(vars, exceptions);} };
