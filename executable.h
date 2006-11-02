// Copyright (C) 2005, 2006 Samuel Newbold

class Executable_t {
 private:
  static Argv_t call_stack;
  static int global_nesting;
  static bool excessive_nesting;
  static bool in_signal_handler;
  unsigned executable_nesting;

 protected:
  int last_return;

 public:
  Executable_t(void) : executable_nesting(0), del_on_term(false) {};
  int last_ret(void) const {return last_return;};
  bool is_running(void) const {return !!executable_nesting;};
  bool del_on_term;

  static const int SIGNONE   =  0;
  static const int SIGEXNEST = -1;
  bool increment_nesting(const Argv_t& argv);
  bool decrement_nesting(const Argv_t& argv);
  static int caught_signal;
  static bool unwind_stack(void) {return caught_signal != SIGNONE;}
  static void signal_handler(void);

  virtual int operator() (const Argv_t& argv) = 0;
  virtual const std::string& name(void) const = 0;
  virtual std::string str() const = 0;};

class Binary_t : public Executable_t {
  std::string implementation;
 public:
  Binary_t(const std::string& impl);
  virtual int operator() (const Argv_t& argv);
  virtual const std::string& name(void) const {return implementation;};
  virtual std::string str() const {return implementation;}; };

class Builtin_t : public Executable_t {
  int (*implementation)(const Argv_t& argv);
  std::string name_v;
 public:
  Builtin_t(const std::string& name, int (*impl)(const Argv_t& argv));
  virtual int operator() (const Argv_t& argv);
  virtual const std::string& name(void) const {return name_v;};
  virtual std::string str() const {return name_v;}; };

