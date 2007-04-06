// Copyright (C) 2005, 2006 Samuel Newbold

class Command_stream_t {
  std::istream& src;
 public:
  Command_stream_t (std::istream& s) : src(s) {};
  Command_stream_t& operator>> (Arg_script_t& dest);
  operator void* () const;
  bool operator! () const; };

