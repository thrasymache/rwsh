// Copyright (C) 2007 Samuel Newbold

struct Rwsh_stream_t {
  virtual Rwsh_stream_t* copy_pointer(void) = 0;
  virtual Rwsh_stream_t* child_stream(void) = 0;
  virtual Rwsh_stream_t& operator<<(const std::string& r) = 0;
  virtual Rwsh_stream_t& operator<<(int r) = 0;
  virtual int fileno(void) = 0;
  virtual void flush(void) = 0;
  virtual std::string str(void) const = 0;};

struct Default_stream_t : public Rwsh_stream_t {
  virtual Rwsh_stream_t* copy_pointer(void) {return this;};
  virtual Rwsh_stream_t* child_stream(void) {return this;};
  virtual Rwsh_stream_t& operator<<(const std::string& r);
  virtual Rwsh_stream_t& operator<<(int r);
  virtual int fileno(void);
  virtual void flush(void);
  virtual std::string str(void) const;};

extern Rwsh_stream_t* default_stream_p;

class Inheirited_stream_t : public Rwsh_stream_t {
  Rwsh_stream_t& parent;
 public:
  Inheirited_stream_t(Rwsh_stream_t& parent_i) : parent(parent_i) {};
  virtual Rwsh_stream_t* copy_pointer(void) {
    return new Inheirited_stream_t(parent);};
  virtual Rwsh_stream_t* child_stream(void) {
    return new Inheirited_stream_t(parent);};
  virtual Rwsh_stream_t& operator<<(const std::string& r);
  virtual Rwsh_stream_t& operator<<(int r);
  virtual int fileno(void);
  virtual void flush(void);
  virtual std::string str(void) const;};
