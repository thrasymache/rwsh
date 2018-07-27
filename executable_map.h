// Copyright (C) 2005-2018 Samuel Newbold

// Executable_map must be able to lookup rwsh.argfunction, which is
// part of an Argm. Thus Argm must be the key_type so as to specify which
// rwsh.argfunction is intended. Argm are not stored in the map, so
// value_type is not std::pair<key_type, mapped_type>
class Executable_map : private std::map<std::string, Base_executable*> {
  typedef std::map<std::string, Base_executable*> Base;
  bool in_autofunction;
  int not_found(Argm& argm, Error_list& exceptions);    // doesn't catch unwind
 public:
  typedef Base::const_iterator const_iterator;

  Executable_map(void);
  const_iterator begin(void) const {return Base::begin();};
  const_iterator end(void) const {return Base::end();};
  size_type erase (const std::string& key);
  Base_executable* find(const Argm& key);
  int run(Argm& argm, Error_list& exceptions);          // doesn't catch unwind
  int base_run(Argm& argm, Error_list& exceptions);           // catches unwind
  void unused_var_check_at_exit(void);                        // catches unwind
  bool run_if_exists(const std::string& key, Argm& argm);     // catches unwind

  // insert executable if not present, replace if executable already exists
  void set(Named_executable* target); };

extern Executable_map executable_map;

