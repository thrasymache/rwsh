// Copyright (C) 2008 Samuel Newbold

class Default_istream : public Rwsh_istream {
  int fd_v;
  std::istream* implementation;

 public:
  Default_istream(int fd_i);
  virtual ~Default_istream(void) {};
  virtual Rwsh_istream* copy_pointer(void);
  virtual bool fail(void);
  virtual Rwsh_istream& getline(std::string& dest);
  virtual int fd(void);
  virtual std::string str(void) const;};

class Default_ostream : public Rwsh_ostream {
  int fd_v;
  std::ostream* implementation;

 public:
  Default_ostream(int fd_i);
  virtual ~Default_ostream(void) {};
  virtual Rwsh_ostream* copy_pointer(void);
  virtual Rwsh_ostream& operator<<(const std::string& r);
  virtual Rwsh_ostream& operator<<(int r);
  virtual Rwsh_ostream& operator<<(unsigned int r);
  virtual Rwsh_ostream& operator<<(double r);
  virtual Rwsh_ostream& operator<<(struct timeval r);
  virtual bool fail(void);
  virtual int fd(void);
  virtual void flush(void);
  virtual std::string str(void) const;};

