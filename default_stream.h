// Copyright (C) 2008 Samuel Newbold

class Default_istream_t : public Rwsh_istream_t {
  int fileno_v;
  std::istream* implementation;

 public:
  Default_istream_t(int fileno_i);
  virtual ~Default_istream_t(void) {};
  virtual Rwsh_istream_t* copy_pointer(void);
  virtual bool fail(void);
  virtual Rwsh_istream_t& getline(std::string& dest);
  virtual int fileno(void);
  virtual std::string str(void) const;};

class Default_ostream_t : public Rwsh_ostream_t {
  int fileno_v;
  std::ostream* implementation;

 public:
  Default_ostream_t(int fileno_i);
  virtual ~Default_ostream_t(void) {};
  virtual Rwsh_ostream_t* copy_pointer(void);
  virtual Rwsh_ostream_t& operator<<(const std::string& r);
  virtual Rwsh_ostream_t& operator<<(int r);
  virtual Rwsh_ostream_t& operator<<(struct timeval r);
  virtual bool fail(void);
  virtual int fileno(void);
  virtual void flush(void);
  virtual std::string str(void) const;};

