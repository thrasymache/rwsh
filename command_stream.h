// Copyright (C) 2005-2018 Samuel Newbold

class Command_stream {
  Rwsh_istream_p& src;
  bool subprompt;
 public:
  Command_stream (Rwsh_istream_p& s, bool subprompt);
  Command_stream& getline(Command_block& dest, Error_list& errors);
  bool fail () const; };
