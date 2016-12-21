//Copyright (C) 2008-2016 Samuel Newbold

class Clock {
  struct timeval last_shell_start, waiting_for_binary_v, waiting_for_shell_v,
    waiting_for_user_v;
  static struct timezone no_timezone_v;

 public:
  struct timezone* no_timezone;

  Clock(void);

  void user_wait(const struct timeval& before, const struct timeval& after);
  void binary_wait(const struct timeval& before, const struct timeval& after);
  const struct timeval& waiting_for_binary() {return waiting_for_binary_v;};
  const struct timeval& waiting_for_shell() {return waiting_for_shell_v;};
  const struct timeval& waiting_for_user() {return waiting_for_user_v;};

  static void timeval_add(struct timeval& lhs, const struct timeval& rhs) {
    lhs.tv_sec += rhs.tv_sec;
    lhs.tv_usec += rhs.tv_usec;
    if (lhs.tv_usec >= 1000000) {lhs.tv_usec -= 1000000; ++lhs.tv_sec;}};
  static struct timeval timeval_sub(const struct timeval& lhs,
                                    const struct timeval& rhs) {
    struct timeval ret;
    ret.tv_sec = lhs.tv_sec - rhs.tv_sec;
    if (lhs.tv_usec >= rhs.tv_usec) ret.tv_usec = lhs.tv_usec - rhs.tv_usec;
    else {--ret.tv_sec; ret.tv_usec = 1000000 + lhs.tv_usec - rhs.tv_usec;}
    return ret;};
};

extern Clock rwsh_clock;

