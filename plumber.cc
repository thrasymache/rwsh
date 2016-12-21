//Copyright (C) 2008-2016 Samuel Newbold

#include <algorithm>
#include <errno.h>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <sys/time.h>
#include <sys/wait.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "clock.h"
#include "executable.h"
#include "plumber.h"

class fd_handler :
      public std::unary_function<std::pair<int, Rwsh_ostream*>, bool> {
  char* buffer;
  size_t buffer_size;

 public:
  fd_handler(char* buffer_i, size_t buffer_size_i) :
    buffer(buffer_i), buffer_size(buffer_size_i) {};
  bool operator()(const std::pair<int, Rwsh_ostream*>& focus) {
    int n = read(focus.first, buffer, buffer_size);
    if (n <= 0 || Named_executable::unwind_stack()) {
      int ret = close(focus.first);
      if (ret) std::cerr <<"failing close " <<focus.first <<"returned " <<ret
                         <<std::endl;
      return true;}
    else {
      *focus.second <<std::string(buffer, n);
      return false;}}; };

void Plumber::after_fork() {
  for (std::vector<int>::const_iterator i = fds_to_close_on_fork.begin();
       i != fds_to_close_on_fork.end(); ++i) {
    int ret = close(*i);
    if (ret) std::cerr <<"failing close " <<*i <<"returned " <<ret <<std::endl;}
  fds_to_close_on_fork.clear();}

void Plumber::proxy_output(int fd, Rwsh_ostream* destination) {
  std::pair<int, Rwsh_ostream*> focus(fd, destination);
  output_handlers.push_back(focus);}

void Plumber::wait(int *ret) {
  for (std::vector<int>::const_iterator i = fds_to_close_on_wait.begin();
       i != fds_to_close_on_wait.end(); ++i) {
    int ret = close(*i);
    if (ret) std::cerr <<"failing close " <<*i <<"returned " <<ret <<std::endl;}
  fds_to_close_on_wait.clear();
  fds_to_close_on_fork.clear();
  for (std::vector<std::pair<int, Rwsh_ostream*> >::iterator
       output_handlers_end = output_handlers.end();
       output_handlers_end != output_handlers.begin();
       output_handlers_end = std::remove_if(output_handlers.begin(),
                                        output_handlers.end(),
                                        fd_handler(buffer, sizeof buffer)));
  output_handlers.clear();
  if (Named_executable::unwind_stack()) return;
  struct timeval before, after;
  gettimeofday(&before, rwsh_clock.no_timezone);
  int wait_return = ::wait(ret);
  gettimeofday(&after, rwsh_clock.no_timezone);
  rwsh_clock.binary_wait(before, after);
  if (wait_return < 0)
    std::cerr <<"failing wait with errno " <<errno <<std::endl;}

