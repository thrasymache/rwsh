// Copyright (C) 2007 Samuel Newbold

class File_istream_t : public Rwsh_istream_t {
  std::string name;
  int file_descriptor;
  FILE* dest;
  bool fail_v;

  void open(void);
 public:
  File_istream_t(const std::string& name_i);
  ~File_istream_t();
  virtual Rwsh_istream_t* copy_pointer() {return new File_istream_t(name);};
  virtual bool fail(void);
  virtual Rwsh_istream_t& getline(std::string& dest_str);
  virtual int fd(void);
  virtual std::string str(void) const;};

class File_ostream_t : public Rwsh_ostream_t {
  std::string name;
  int file_descriptor;
  FILE* dest;

  void open(void);
 public:
  File_ostream_t(const std::string& name_i);
  ~File_ostream_t();
  virtual Rwsh_ostream_t* copy_pointer() {return new File_ostream_t(name);};
  virtual Rwsh_ostream_t& operator<<(const std::string& r);
  virtual Rwsh_ostream_t& operator<<(int r);
  virtual Rwsh_ostream_t& operator<<(struct timeval r);
  virtual bool fail(void);
  virtual int fd(void);
  virtual void flush(void);
  virtual std::string str(void) const;};
