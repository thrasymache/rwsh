// Copyright (C) 2018 Samuel Newbold

struct Pipe_istream : public Rwsh_istream {
  Pipe_istream(int fd_i);
  void late_init(int fd_i);
  virtual ~Pipe_istream(void) {};
  virtual Rwsh_istream* copy_pointer(void);
  virtual Rwsh_istream& getline(std::string& dest);
  virtual int fd(void) {return fd_v;};

  virtual std::string str(void) const;};

