// Copyright (C) 2007 Samuel Newbold

class Substitution_stream_t : public Rwsh_stream_t {
  std::ostringstream buffer;

 public:
  virtual Rwsh_stream_t* copy_pointer(void);
  virtual Rwsh_stream_t* child_stream(void) {
    return new Inheirited_stream_t(*this);}
  virtual Rwsh_stream_t& operator<<(const std::string& r);
  virtual Rwsh_stream_t& operator<<(int r);
  virtual int fileno(void);
  virtual void flush(void) {};
  virtual std::string str(void) const;

  std::string value(void) const {return buffer.str();}; };
