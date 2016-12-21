// Copyright (C) 2005-2016 Samuel Newbold

// Executable_map must be able to lookup rwsh.argfunction, which is
// part of an Argm. Thus Argm must be the key_type so as to specify which
// rwsh.argfunction is intended. Argm are not stored in the map, so
// value_type is not std::pair<key_type, mapped_type>
class Executable_map : private std::map<std::string, Base_executable*> {
  typedef std::map<std::string, Base_executable*> Base;
  bool in_autofunction;
  int not_found(Argm& argm);                    // doesn't catch unwind_stack
 public:
  Executable_map(void);
  size_type erase (const std::string& key);
  Base_executable* find(const Argm& key);
  int run(Argm& argm, std::list<Argm>& exceptions);     // doesn't catch unwind
  int base_run(Argm& argm);                                   // catches unwind
  bool run_if_exists(const std::string& key, Argm& argm);     // catches unwind

  // insert executable if not present, replace if executable already exists
  void set(Named_executable* target); };

extern Executable_map executable_map;

