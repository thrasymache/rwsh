// Copyright (C) 2005, 2006, 2007 Samuel Newbold

class Executable {
 private:
  static int global_nesting;
  static bool excessive_nesting;
  static bool in_signal_handler;
  unsigned executable_nesting;

 protected:
  int last_return;
  unsigned execution_count_v;
  struct timeval last_execution_time_v;
  struct timeval total_execution_time_v;
  static bool caught_signal;

 public:
  bool del_on_term;

  Executable(void) : executable_nesting(0), last_return(0),
    execution_count_v(0), del_on_term(false) {
    last_execution_time_v.tv_sec = 0;
    last_execution_time_v.tv_usec = 0;
    total_execution_time_v.tv_sec = 0;
    total_execution_time_v.tv_usec = 0;};
  int last_ret(void) const {return last_return;};
  unsigned execution_count(void) const {return execution_count_v;};
  struct timeval last_execution_time(void) const {
    return last_execution_time_v;};
  struct timeval total_execution_time(void) const {
    return total_execution_time_v;};
  bool is_running(void) const {return !!executable_nesting;};

  bool increment_nesting(const Argv& argv);
  bool decrement_nesting(const Argv& argv);
  static Argv call_stack;
  static bool unwind_stack(void) {return caught_signal;}
  static void signal_handler(void);
  static void unix_signal_handler(int sig);

  virtual int operator() (const Argv& argv) = 0;
  virtual const std::string& name(void) const = 0;
  virtual std::string str() const = 0;};

class Binary : public Executable {
  std::string implementation;
 public:
  Binary(const std::string& impl);
  virtual int operator() (const Argv& argv);
  virtual const std::string& name(void) const {return implementation;};
  virtual std::string str() const {return implementation;}; };

class Builtin : public Executable {
  int (*implementation)(const Argv& argv);
  std::string name_v;
 public:
  Builtin(const std::string& name, int (*impl)(const Argv& argv));
  virtual int operator() (const Argv& argv);
  virtual const std::string& name(void) const {return name_v;};
  virtual std::string str() const {return name_v;}; };

