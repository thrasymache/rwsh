// Copyright (C) 2008 Samuel Newbold

class Plumber {
  std::vector<std::pair<Rwsh_istream*, Rwsh_ostream*> > output_handlers;
  std::vector<Rwsh_istream*> to_close_on_fork;
  std::vector<Rwsh_ostream*> to_close_on_wait;
  char buffer[512];

  void run_handlers(void);
 public:
  void after_fork();
  void proxy_output(Rwsh_istream* src, Rwsh_ostream* dest);
  void wait(int* ret);};

extern Plumber plumber;
