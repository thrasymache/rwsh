// Copyright (C) 2008 Samuel Newbold

class Plumber {
  std::vector<std::pair<int, Rwsh_ostream*> > output_handlers;
  std::vector<int> fds_to_close_on_fork;
  std::vector<int> fds_to_close_on_wait;
  char buffer[512];

  void run_handlers(void);
 public:
  void after_fork();
  void close_on_fork(int fd) {fds_to_close_on_fork.push_back(fd);};
  void close_on_wait(int fd) {fds_to_close_on_wait.push_back(fd);};
  void proxy_output(int fd, Rwsh_ostream* destination);
  void wait(int* ret);};

extern Plumber plumber;
