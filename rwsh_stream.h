// Copyright (C) 2007 Samuel Newbold

struct Rwsh_istream_t {
  virtual ~Rwsh_istream_t(void) {};
  virtual Rwsh_istream_t* copy_pointer(void) = 0;
  virtual bool fail(void) = 0;
  virtual Rwsh_istream_t& getline(std::string& dest) = 0;
  virtual int fd(void) = 0;
  virtual std::string str(void) const = 0;};

class Rwsh_istream_p {
  Rwsh_istream_t* implementation;
  bool inherited;
  bool is_default_v;

 public:
  Rwsh_istream_p(Rwsh_istream_t* imp, bool inherited_i, bool is_default_i) : 
    implementation(imp), inherited(inherited_i), is_default_v(is_default_i) {};
  Rwsh_istream_p(const Rwsh_istream_p& src);
  Rwsh_istream_p child_stream(void) const;
  Rwsh_istream_p& operator=(const Rwsh_istream_p& src);
  ~Rwsh_istream_p(void);
  virtual bool fail(void) {return implementation->fail();}
  virtual Rwsh_istream_t& getline(std::string& dest) {
    return implementation->getline(dest);};
  int fd(void) {return implementation->fd();};
  std::string str(void) const {return implementation->str();};
  bool is_default(void) const {return is_default_v;}; };

struct Rwsh_ostream_t {
  virtual ~Rwsh_ostream_t(void) {};
  virtual Rwsh_ostream_t* copy_pointer(void) = 0;
  virtual Rwsh_ostream_t& operator<<(const std::string& r) = 0;
  virtual Rwsh_ostream_t& operator<<(int r) = 0;
  virtual Rwsh_ostream_t& operator<<(struct timeval r) = 0;
  virtual bool fail(void) = 0;
  virtual int fd(void) = 0;
  virtual void flush(void) = 0;
  virtual std::string str(void) const = 0;};

class Rwsh_ostream_p {
  Rwsh_ostream_t* implementation;
  bool inherited;
  bool is_default_v;

 public:
  Rwsh_ostream_p(Rwsh_ostream_t* imp, bool inherited_i, bool is_default_i) : 
    implementation(imp), inherited(inherited_i), is_default_v(is_default_i) {};
  Rwsh_ostream_p(const Rwsh_ostream_p& src);
  Rwsh_ostream_p child_stream(void) const;
  Rwsh_ostream_p& operator=(const Rwsh_ostream_p& src);
  ~Rwsh_ostream_p(void);
  Rwsh_ostream_p& operator<<(const std::string& r) {
    *implementation <<r;
    return *this;};
  Rwsh_ostream_p& operator<<(int r) {
    *implementation <<r;
    return *this;};
  Rwsh_ostream_p& operator<<(struct timeval r) {
    *implementation <<r;
    return *this;};
  virtual bool fail(void) {return implementation->fail();}
  int fd(void) {return implementation->fd();};
  void flush(void) {implementation->flush();};
  std::string str(void) const {return implementation->str();};
  bool is_default(void) const {return is_default_v;}; };

extern Rwsh_istream_p default_input;
extern Rwsh_ostream_p default_output, default_error;

