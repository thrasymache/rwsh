// Copyright (C) 2007 Samuel Newbold

class Substitution_stream_t : public Rwsh_ostream_t {
  std::ostringstream buffer;

 public:
  virtual Rwsh_ostream_t* copy_pointer(void);
  virtual Rwsh_ostream_t& operator<<(const std::string& r);
  virtual Rwsh_ostream_t& operator<<(int r);
  virtual Rwsh_ostream_t& operator<<(struct timeval r);
  virtual bool fail(void);
  virtual int fd(void);
  virtual void flush(void) {};
  virtual std::string str(void) const;

  Rwsh_ostream_p child_stream(void) {return Rwsh_ostream_p(this, true, false);};
  std::string value(void) const {return buffer.str();}; };
