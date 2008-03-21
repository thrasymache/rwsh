// Copyright (C) 2005, 2006, 2007 Samuel Newbold

class Command_stream_t {
  std::istream& src;
  struct timeval before_input, after_input;
  static struct timeval waiting_for_binary_v, waiting_for_shell_v,
    waiting_for_user_v;
 public:
  Command_stream_t (std::istream& s);
  Command_stream_t& operator>> (Arg_script_t& dest);
  operator void* () const;
  bool operator! () const;
  static const struct timeval& waiting_for_binary(void) {
    return waiting_for_binary_v;};
  static const struct timeval& waiting_for_shell(void) {
    return waiting_for_shell_v;};
  static const struct timeval& waiting_for_user(void) {
    return waiting_for_user_v;}; };

