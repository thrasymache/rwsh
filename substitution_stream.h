// Copyright (C) 2007 Samuel Newbold

class Substitution_stream_t : public Rwsh_stream_t {
  std::ostringstream buffer;

 public:
  virtual Rwsh_stream_t* copy_pointer(void);
  virtual Rwsh_stream_t& operator<<(const std::string& r);
  virtual Rwsh_stream_t& operator<<(int r);
  virtual int fileno(void);
  virtual void flush(void) {};
  virtual std::string str(void) const;

  Rwsh_stream_p child_stream(void) {return Rwsh_stream_p(this, true);};
  std::string value(void) const {return buffer.str();}; };
