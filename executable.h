// Copyright (C) 2005-2016 Samuel Newbold

class Base_executable {
  unsigned executable_nesting;

 protected:
  static int global_nesting;
  static bool in_exception_handler;
  static bool unwind_stack_v;
  static bool collect_excess_thrown;
  static unsigned current_exception_count;
  static unsigned dropped_catches;
  static bool execution_handler_excess_thrown;

 public:
  static Argm::Exception_t caught_signal;
  bool del_on_term;
  unsigned execution_count_v;
  int last_return;
  std::string last_exception_v;
  struct timeval last_execution_time_v;
  static unsigned max_collect;
  static unsigned max_extra;
  static unsigned max_nesting;
  struct timeval total_execution_time_v;

  Base_executable(void) : executable_nesting(0), del_on_term(false),
      last_return(0), execution_count_v(0) {
    last_execution_time_v.tv_sec = 0;
    last_execution_time_v.tv_usec = 0;
    total_execution_time_v.tv_sec = 0;
    total_execution_time_v.tv_usec = 0;};
  bool is_running(void) const {return !!executable_nesting;};
  static void exception_handler(std::list<Argm>& exceptions);
  static void catch_blocks(const Argm& argm, std::list<Argm>& exceptions);
  static void add_error(std::list<Argm>& exceptions, const Argm& error);
  unsigned execution_count(void) const {return execution_count_v;};
  const std::string& last_exception(void) const {return last_exception_v;};
  struct timeval last_execution_time(void) const {
    return last_execution_time_v;};
  int last_ret(void) const {return last_return;};
  struct timeval total_execution_time(void) const {
    return total_execution_time_v;};
  static void unix_signal_handler(int sig);
  static bool unwind_stack(void) {return unwind_stack_v;}

  int operator() (const Argm& argm, std::list<Argm>& parent_exceptions);
  virtual int execute(const Argm& argm, std::list<Argm>& exceptions) const = 0;
  virtual std::string str() const = 0;};

class Named_executable : public Base_executable {
  friend class Executable_map;

 public:
  Named_executable(void) {};
  virtual const std::string& name(void) const = 0; };

class Binary : public Named_executable {
  std::string implementation;
 public:
  Binary(const std::string& impl);
  virtual int execute(const Argm& argm, std::list<Argm>& exceptions) const;
  virtual const std::string& name(void) const {return implementation;};
  virtual std::string str() const {return implementation;}; };

class Builtin : public Named_executable {
  int (*implementation)(const Argm& argm, std::list<Argm>& exceptions);
  std::string name_v;
 public:
  Builtin(const std::string& name_i,
          int (*impl)(const Argm& argm, std::list<Argm>& exceptions));
  virtual int execute(const Argm& argm, std::list<Argm>& exceptions) const;
  virtual const std::string& name(void) const {return name_v;};
  virtual std::string str() const {return name_v;}; };

