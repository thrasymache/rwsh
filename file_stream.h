// Copyright (C) 2007 Samuel Newbold

class File_istream : public Rwsh_istream {
  std::string name;
  int file_descriptor;
  FILE* dest;
  bool fail_v;

  void open(void);
 public:
  File_istream(const std::string& name_i);
  ~File_istream();
  virtual Rwsh_istream* copy_pointer() {return new File_istream(name);};
  virtual bool fail(void);
  virtual Rwsh_istream& getline(std::string& dest_str);
  virtual int fd(void);
  virtual std::string str(void) const;};

class File_ostream : public Rwsh_ostream {
  std::string name;
  int file_descriptor;
  FILE* dest;

  void open(void);
 public:
  File_ostream(const std::string& name_i);
  ~File_ostream();
  virtual Rwsh_ostream* copy_pointer() {return new File_ostream(name);};
  virtual Rwsh_ostream& operator<<(const std::string& r);
  virtual Rwsh_ostream& operator<<(int r);
  virtual Rwsh_ostream& operator<<(unsigned int r);
  virtual Rwsh_ostream& operator<<(double r);
  virtual Rwsh_ostream& operator<<(struct timeval r);
  virtual bool fail(void);
  virtual int fd(void);
  virtual void flush(void);
  virtual std::string str(void) const;};
