//Copyright (C) 2008-2017 Samuel Newbold

#include <algorithm>
#include <errno.h>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sys/time.h>
#include <sys/wait.h>
#include <string>
#include <vector>

#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "call_stack.h"
#include "clock.h"
#include "plumber.h"

class read_handler :
      public std::function<bool (std::pair<Rwsh_istream*, Rwsh_ostream*>)> {
  char* buffer;
  size_t buffer_size;

 public:
  read_handler(char* buffer_i, size_t buffer_size_i) :
    buffer(buffer_i), buffer_size(buffer_size_i) {};
  bool operator()(const std::pair<Rwsh_istream*, Rwsh_ostream*>& focus) {
    int n = focus.first->read(buffer, buffer_size);
    if (n <= 0 || global_stack.unwind_stack()) {
      focus.first->close();
      return true;}
    else {
      *focus.second <<std::string(buffer, n);
      return false;}}; };

void Plumber::after_fork() {
  for (auto i = to_close_on_fork.begin(); i != to_close_on_fork.end(); ++i)
    (*i)->close();
  to_close_on_fork.clear();}

void Plumber::proxy_output(Rwsh_istream* src, Rwsh_ostream* dest) {
  to_close_on_fork.push_back(src);
  to_close_on_wait.push_back(dest);
  std::pair<Rwsh_istream*, Rwsh_ostream*> focus(src, dest);
  output_handlers.push_back(focus);}

void Plumber::wait(int *ret) {
  for (auto i = to_close_on_wait.begin(); i != to_close_on_wait.end(); ++i)
    (*i)->close();
  to_close_on_wait.clear();
  to_close_on_fork.clear();
  for (auto output_handlers_end = output_handlers.end();
       output_handlers_end != output_handlers.begin();
       output_handlers_end = std::remove_if(output_handlers.begin(),
                                        output_handlers.end(),
                                        read_handler(buffer, sizeof buffer)));
  output_handlers.clear();
  if (global_stack.unwind_stack()) return;
  struct timeval before, after;
  gettimeofday(&before, rwsh_clock.no_timezone);
  int wait_return = ::wait(ret);
  gettimeofday(&after, rwsh_clock.no_timezone);
  rwsh_clock.binary_wait(before, after);
  if (wait_return < 0)
    std::cerr <<"failing wait with errno " <<errno <<std::endl;}

