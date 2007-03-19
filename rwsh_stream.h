// Copyright (C) 2007 Samuel Newbold

struct Rwsh_stream_t {
  virtual Rwsh_stream_t& operator<<(const std::string& r) = 0;
  virtual Rwsh_stream_t& operator<<(int r) = 0;
  virtual void flush(void) = 0;};

struct Default_stream_t : public Rwsh_stream_t {
  Rwsh_stream_t& operator<<(const std::string& r);
  Rwsh_stream_t& operator<<(int r);
  void flush(void);};
