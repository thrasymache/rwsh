// Copyright (C) 2007 Samuel Newbold

class File_stream_t : public Rwsh_stream_t {
  std::ofstream dest;
 public:
  File_stream_t(const std::string& name);
  virtual Rwsh_stream_t& operator<<(const std::string& r);
  virtual Rwsh_stream_t& operator<<(int r);
  virtual void flush(void);};
