// Copyright (C) 2019 Samuel Newbold

class Call_stack {
  bool collect_excess_thrown;
  bool execution_handler_excess_thrown;
  bool in_exception_handler_v;
  bool unwind_stack_v;
  int exit_v;

 public:
  Argm::Exception_t caught_signal;
  unsigned current_exception_count;
  bool exit_requested;
  int global_nesting;
  unsigned max_collect;
  unsigned max_extra;
  int max_nesting;

  Call_stack(void) : caught_signal(Argm::No_exception),
    collect_excess_thrown(false), current_exception_count(0),
    execution_handler_excess_thrown(false), exit_requested(false),
    global_nesting(0), in_exception_handler_v(false), exit_v(0),
    max_collect(1), max_extra(1), max_nesting(0), unwind_stack_v(false) {}
  void add_error(void);
  void catch_blocks(const Argm& argm, Error_list& exceptions);
  void catch_one(Argm& argm, Error_list& exceptions);
  int collect_errors_core(const Argm& argm, bool logic, Error_list& parent);
  void exception_handler(Error_list& exceptions);
  int exit_value(void) {return exit_v;}
  bool in_exception_handler(void) {return in_exception_handler_v;}
  bool remove_exceptions(const std::string &name, Error_list& exceptions);
  void replace_error(void);
  void request_exit(int exit_val);
  void reset(void);
  bool unwind_stack(void) {return unwind_stack_v;}
};

extern Call_stack global_stack;

struct Conditional_state {
  bool in_if_block;
  bool successful_condition;
  bool exception_thrown;
  Conditional_state(void);};

extern Conditional_state gc_state;
