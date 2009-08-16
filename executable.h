// Copyright (C) 2005, 2006, 2007 Samuel Newbold

class Executable_t {
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

 public:
  bool del_on_term;

  Executable_t(void) : executable_nesting(0), last_return(0),
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

  static const int SIGNONE     =  0;
  static const int SIGEXNEST   = -1;
  static const int SIGVAR      = -2;
  static const int SIGFILE     = -3;
  static const int SIGSUB      = -4;
  static const int SIGNOEXEC   = -5;
  static const int SIGRANGE    = -6;
  static const int SIGRESRANGE = -7;
  static const int SIGNAN      = -8;
  static const int SIGDIVZERO  = -9;
  bool increment_nesting(const Argv_t& argv);
  bool decrement_nesting(const Argv_t& argv);
  static int caught_signal;
  static Argv_t call_stack;
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

