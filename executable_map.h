// Copyright (C) 2005, 2006, 2007 Samuel Newbold

// Executable_map must be able to lookup rwsh.argfunction, which is
// part of an Argv. Thus Argv must be the key_type so as to specify which
// rwsh.argfunction is intended. Argv are not stored in the map, so
// value_type is not std::pair<key_type, mapped_type>
class Executable_map : private std::map<std::string, Executable*> {
  typedef std::map<std::string, Executable*> Base;
  bool in_autofunction;
  int not_found(Argv& argv);                    // doesn't catch unwind_stack
 public:
  Executable_map(void);
  size_type erase (const std::string& key);
  Executable* find(const Argv& key);
  int run(Argv& argv);                          // doesn't catch unwind_stack
  bool run_if_exists(const std::string& key, Argv& argv); // catches unwind

  // insert executable if not present, replace if executable already exists
  void set(Executable* target); };

extern Executable_map executable_map;

