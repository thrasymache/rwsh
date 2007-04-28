// Copyright (C) 2007 Samuel Newbold

class Substitution_stream_t : public Rwsh_stream_t {
  std::ostringstream buffer;

 public:
  Rwsh_stream_t& operator<<(const std::string& r);
  Rwsh_stream_t& operator<<(int r);
  void flush(void) {};

  std::string str(void) const {return buffer.str();}; };
