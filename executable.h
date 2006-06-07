// Copyright (C) 2005, 2006 Samuel Newbold

class Executable_t {
 protected:
  int last_return;
  unsigned current_nesting;
  static int global_nesting;
  static bool in_excessive_nesting_handler;
  static Argv_t call_stack;

 public:
  Executable_t(void) : current_nesting(0), del_on_term(false) {};
  int last_ret(void) const {return last_return;};
  bool is_running(void) const {return !!current_nesting;};
  bool del_on_term;

  static bool excessive_nesting;
  static bool increment_nesting(void);
  static bool decrement_nesting(void);
  static void excessive_nesting_handler(const Argv_t& src_argv);

  virtual int operator() (const Argv_t& argv) = 0;
  virtual std::string name(void) const = 0;
  virtual std::string str() const = 0;};

class Binary_t : public Executable_t {
  std::string implementation;
 public:
  Binary_t(const std::string& impl);
  virtual int operator() (const Argv_t& argv);
  virtual std::string name(void) const {return implementation;};
  virtual std::string str() const {return implementation;}; };

class Built_in_t : public Executable_t {
  int (*implementation)(const Argv_t& argv);
  std::string name_v;
 public:
  Built_in_t(const std::string& name, int (*impl)(const Argv_t& argv));
  virtual int operator() (const Argv_t& argv);
  virtual std::string name(void) const {return name_v;};
  virtual std::string str() const {return name_v;}; };

