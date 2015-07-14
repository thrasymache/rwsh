// Copyright (C) 2005-2015 Samuel Newbold

class Base_executable {
  unsigned executable_nesting;

 protected:
  static int global_nesting;
  static bool in_signal_handler;
  static Argm::Sig_type caught_signal;

 public:
  bool del_on_term;
  static Argm call_stack;

  Base_executable(void) : executable_nesting(0), del_on_term(false) {};
  bool decrement_nesting(const Argm& argm);
  bool increment_nesting(const Argm& argm);
  bool is_running(void) const {return !!executable_nesting;};
  static void signal_handler(void);
  static void unix_signal_handler(int sig);
  static Argm::Sig_type unwind_stack(void) {return caught_signal;}

  virtual int operator() (const Argm& argm) = 0;
  virtual std::string str() const = 0;};

class Named_executable : public Base_executable {
  friend class Executable_map;

 protected:
  int last_return;
  unsigned execution_count_v;
  struct timeval last_execution_time_v;
  struct timeval total_execution_time_v;

 public:
  Named_executable(void) : last_return(0), execution_count_v(0) {
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

  virtual const std::string& name(void) const = 0; };

class Binary : public Named_executable {
  std::string implementation;
 public:
  Binary(const std::string& impl);
  virtual int operator() (const Argm& argm);
  virtual const std::string& name(void) const {return implementation;};
  virtual std::string str() const {return implementation;}; };

class Builtin : public Named_executable {
  int (*implementation)(const Argm& argm);
  std::string name_v;
 public:
  Builtin(const std::string& name, int (*impl)(const Argm& argm));
  virtual int operator() (const Argm& argm);
  virtual const std::string& name(void) const {return name_v;};
  virtual std::string str() const {return name_v;}; };

