// Copyright (C) 2007 Samuel Newbold

class Inheirited_stream_t;

class File_stream_t : public Rwsh_stream_t {
  std::string name;
  int file_descriptor;
  FILE* dest;

  void open(void);
 public:
  File_stream_t(const std::string& name_i);
  ~File_stream_t();
  virtual Rwsh_stream_t* copy_pointer() {return new File_stream_t(name);};
  virtual Rwsh_stream_t& operator<<(const std::string& r);
  virtual Rwsh_stream_t& operator<<(int r);
  virtual int fileno(void);
  virtual void flush(void);
  virtual std::string str(void) const;};
