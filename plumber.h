// Copyright (C) 2008 Samuel Newbold

class Plumber {
  std::vector<std::pair<int, Rwsh_ostream_t*> > output_handlers;
  std::vector<int> filedes_to_close;
  char buffer[512];

  void run_handlers(void);
 public:
  void proxy_output(int fileno, Rwsh_ostream_t* destination);
  void close_on_wait(int filedes);
  void wait(int* ret);};

extern Plumber plumber;
