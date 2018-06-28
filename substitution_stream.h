// Copyright (C) 2007 Samuel Newbold

class Substitution_stream : public Rwsh_ostream {
  std::ostringstream buffer;
  Pipe_istream in_pipe;

 public:
  Substitution_stream(void) : Rwsh_ostream(-1), in_pipe(-1) {};
  virtual ~Substitution_stream(void) {};
  virtual Rwsh_ostream* copy_pointer(void);
  virtual Rwsh_ostream& operator<<(const std::string& r);
  virtual Rwsh_ostream& operator<<(int r);
  virtual Rwsh_ostream& operator<<(unsigned int r);
  virtual Rwsh_ostream& operator<<(double r);
  virtual Rwsh_ostream& operator<<(struct timeval r);
  virtual int fd(void);
  virtual void flush(void) {};
  virtual std::string str(void) const;

  Rwsh_ostream_p child_stream(void) {return Rwsh_ostream_p(this, true, false);};
  std::string value(void) const {return buffer.str();}; };
