// Copyright (C) 2005-2018 Samuel Newbold

class Base_executable {
  unsigned executable_nesting;

 public:
  bool del_on_term;
  unsigned execution_count_v;
  std::string last_exception_v;
  struct timeval last_execution_time_v;
  struct timeval total_execution_time_v;

  Base_executable(void) : executable_nesting(0), del_on_term(false),
      execution_count_v(0) {
    last_execution_time_v.tv_sec = 0;
    last_execution_time_v.tv_usec = 0;
    total_execution_time_v.tv_sec = 0;
    total_execution_time_v.tv_usec = 0;};
  virtual ~Base_executable(void) {}
  bool is_running(void) const {return !!executable_nesting;};
  unsigned execution_count(void) const {return execution_count_v;};
  const std::string& last_exception(void) const {return last_exception_v;};
  struct timeval last_execution_time(void) const {
    return last_execution_time_v;};
  struct timeval total_execution_time(void) const {
    return total_execution_time_v;};

  void operator() (const Argm& argm, Error_list& parent_exceptions);
  virtual void execute(const Argm& argm, Error_list& exceptions) const = 0;
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
  virtual void execute(const Argm& argm, Error_list& exceptions) const;
  virtual const std::string& name(void) const {return implementation;};
  virtual std::string str() const {return implementation;}; };
