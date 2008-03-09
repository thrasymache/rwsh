//Copyright (C) 2008 Samuel Newbold

#include <algorithm>
#include <functional>
#include <iostream>
#include <sys/wait.h>
#include <string>
#include <vector>

#include "rwsh_stream.h"

#include "argv.h"
#include "executable.h"
#include "plumber.h"

Plumber plumber;

class filedes_handler : 
      public std::unary_function<std::pair<int, Rwsh_ostream_t*>, bool> {
  char* buffer;
  size_t buffer_size;

 public:
  filedes_handler(char* buffer_i, size_t buffer_size_i) :
    buffer(buffer_i), buffer_size(buffer_size_i) {};
  bool operator()(const std::pair<int, Rwsh_ostream_t*>& focus) {
    int n = read(focus.first, buffer, buffer_size);
    if (n <= 0) {
      int ret = ::close(focus.first);
      if (ret) std::cerr <<"failing close " <<focus.first <<"returned " <<ret 
                         <<std::endl;
      return true;}
    else {
      *focus.second <<std::string(buffer, n);
      return false;}}; };
    
void Plumber::proxy_output(int fileno, Rwsh_ostream_t* destination) {
  std::pair<int, Rwsh_ostream_t*> focus(fileno, destination);
  output_handlers.push_back(focus);}

void Plumber::close_on_wait(int filedes) {filedes_to_close.push_back(filedes);}

void Plumber::wait(int *ret) {
  for (std::vector<int>::const_iterator i = filedes_to_close.begin();
       i != filedes_to_close.end(); ++i) {
    int ret = ::close(*i);
    if (ret) std::cerr <<"failing close " <<*i <<"returned " <<ret <<std::endl;}
  filedes_to_close.clear();
  for (std::vector<std::pair<int, Rwsh_ostream_t*> >::iterator 
       output_handlers_end = output_handlers.end();
       output_handlers_end != output_handlers.begin();
       output_handlers_end = std::remove_if(output_handlers.begin(),
                                        output_handlers.end(),
                                        filedes_handler(buffer, sizeof buffer)))
    if (Executable_t::caught_signal) return;
  output_handlers.clear();
  if (::wait(ret) < 0)
    std::cerr <<"failing wait with errno " <<errno <<std::endl;}

