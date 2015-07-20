// Copyright (C) 2005, 2006, 2007 Samuel Newbold

class Command_stream {
  std::istream& src;
  bool subprompt;
 public:
  Command_stream (std::istream& s, bool subprompt);
  Command_stream& operator>> (Arg_script& dest);
  operator void* () const;
  bool operator! () const; };

