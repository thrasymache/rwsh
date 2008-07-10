// Copyright (C) 2005, 2006, 2007 Samuel Newbold

class Command_stream_t {
  std::istream& src;
  bool subprompt;
 public:
  Command_stream_t (std::istream& s, bool subprompt);
  Command_stream_t& operator>> (Arg_script_t& dest);
  operator void* () const;
  bool operator! () const; };

