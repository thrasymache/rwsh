// Copyright (C) 2007 Samuel Newbold

struct Rwsh_stream_t {
  virtual Rwsh_stream_t* copy_pointer(void) = 0;
  virtual Rwsh_stream_t& operator<<(const std::string& r) = 0;
  virtual Rwsh_stream_t& operator<<(int r) = 0;
  virtual int fileno(void) = 0;
  virtual void flush(void) = 0;
  virtual std::string str(void) const = 0;};

class Rwsh_stream_p {
  Rwsh_stream_t* implementation;
  bool inherited;

 public:
  Rwsh_stream_p(void);
  Rwsh_stream_p(Rwsh_stream_t* imp, bool inherited_i) : 
    implementation(imp), inherited(inherited_i) {};
  Rwsh_stream_p(const Rwsh_stream_p& src);
  Rwsh_stream_p child_stream(void) const;
  Rwsh_stream_p& operator=(const Rwsh_stream_p& src);
  ~Rwsh_stream_p(void);
  Rwsh_stream_p& operator<<(const std::string& r) {
    *implementation << r;
    return *this;};
  Rwsh_stream_p& operator<<(int r) {
    *implementation << r;
    return *this;};
  int fileno(void) {return implementation->fileno();};
  void flush(void) {implementation->flush();};
  std::string str(void) const {return implementation->str();};
  bool is_default(void) const;};

