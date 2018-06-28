// Copyright (C) 2008-2018 Samuel Newbold

struct Default_istream : public Rwsh_istream {
  Default_istream(int fd_i);
  virtual ~Default_istream(void) {};
  virtual Rwsh_istream* copy_pointer(void);
  virtual Rwsh_istream& getline(std::string& dest);
  Rwsh_istream& readline_getline(std::string& dest);
  virtual int fd(void);

  virtual std::string str(void) const;};

class Default_ostream : public Rwsh_ostream {
  std::ostream* cpp_style;  // usage is ultimately incompatible with fd

 public:
  Default_ostream(int fd_i);
  virtual ~Default_ostream(void) {};
  virtual Rwsh_ostream* copy_pointer(void);
  virtual Rwsh_ostream& operator<<(const std::string& r);
  virtual Rwsh_ostream& operator<<(int r);
  virtual Rwsh_ostream& operator<<(unsigned int r);
  virtual Rwsh_ostream& operator<<(double r);
  virtual Rwsh_ostream& operator<<(struct timeval r);
  virtual int fd(void);
  virtual void flush(void);
  virtual std::string str(void) const;};

