//Copyright (C) 2008 Samuel Newbold

#include <sys/time.h>

#include "clock.h"

Clock::Clock() {
  no_timezone = &no_timezone_v;
  gettimeofday(&last_shell_start, no_timezone);
  waiting_for_binary_v.tv_sec = 0;
  waiting_for_binary_v.tv_usec = 0;
  waiting_for_shell_v.tv_sec = 0;
  waiting_for_shell_v.tv_usec = 0;
  waiting_for_user_v.tv_sec = 0;
  waiting_for_user_v.tv_usec = 0;}

void Clock::user_wait(const struct timeval& before,
                      const struct timeval& after) {
  timeval_add(waiting_for_shell_v, timeval_sub(before, last_shell_start));
  timeval_add(waiting_for_user_v, timeval_sub(after, before));
  last_shell_start = after;}

void Clock::binary_wait(const struct timeval& before,
                        const struct timeval& after) {
  timeval_add(waiting_for_shell_v, timeval_sub(before, last_shell_start));
  timeval_add(waiting_for_binary_v, timeval_sub(after, before));
  last_shell_start = after;}

