// Copyright (C) 2005, 2006 Samuel Newbold

// Executable_map_t must be able to lookup rwsh.argfunction, which is
// part of an Argv_t. Thus Argv_t must be the key_type so as to specify which
// rwsh.argfunction is intended. Argv_t are not stored in the map, so
// value_type is not std::pair<key_type, mapped_type>
class Executable_map_t : private std::map<std::string, Executable_t*> {
  typedef std::map<std::string, Executable_t*> Base;
 public:
  Executable_map_t(void);
  size_type erase (const std::string& key);
  Executable_t* find(const Argv_t& key);
  int run(Argv_t& argv);
  bool run_if_exists(const std::string& key, Argv_t& argv);
  // insert executable if not present, replace if executable already exists
  void set(Executable_t* target); };

extern Executable_map_t executable_map;

