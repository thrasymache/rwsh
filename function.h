// Copyright (C) 2005-2016 Samuel Newbold

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
                unsigned max_soon);
  Command_block* copy_pointer(void) const {
    if (!this) return 0;
    else {
      Command_block* result = new Command_block(*this);
      return result;}};
  Command_block* apply(const Argm& argm, unsigned nesting,
                       std::list<Argm>& exceptions) const;

  int collect_errors_core(const Argm& src_argm,
                          const std::vector<std::string>& exceptional,
                          bool logic,
                          std::list<Argm>& exceptions);
  virtual int execute(const Argm& argm, std::list<Argm>& exceptions) const;
  void promote_soons(unsigned nesting);
  int prototype_execute(const Argm& argm, const Prototype& prototype,
                        std::list<Argm>& exceptions) const;
  std::string str() const; };

class Function : public Named_executable {
  std::string name_v;
  Prototype prototype;

  Function(const std::string& name_i) :
      name_v(name_i), prototype(true), body() {};
  void check_for_duplicates(const std::string& name);
 public:
  Command_block body;

  Function(const std::string& name, const std::string& src,
           std::string::size_type& point, unsigned max_soon);
  Function(const std::string& name, const std::string& src);
  Function(const Function& src) :
    name_v(src.name_v), prototype(src.prototype), body(src.body) {};
  Function(const std::string& name_i, Argm::const_iterator first_parameter,
           Argm::const_iterator parameter_end, bool non_prototype_i,
           const Command_block& src);
  Function* copy_pointer(void) const {
    if (!this) return 0;
    else return new Function(*this);};
  virtual int execute(const Argm& argm, std::list<Argm>& exceptions) const;
  const std::string& name(void) const {return name_v;};
  void promote_soons(unsigned nesting);
  std::string str() const;};

struct Unclosed_brace {
  std::string prefix;
  Unclosed_brace(const std::string& prefix_i) : prefix(prefix_i) {}};

