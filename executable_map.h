// Copyright (C) 2005, 2006 Samuel Newbold

// Executable_map_t must be able to lookup rwsh.argfunction, which is
// part of an Argv_t. Thus Argv_t must be the key_type so as to specify which
// rwsh.argfunction is intended. Argv_t are not stored in the map, so
// value_type is not std::pair<key_type, mapped_type>
class Executable_map_t : private std::map<std::string, Executable_t*> {
  typedef std::map<std::string, Executable_t*> Base;
 public:
  Executable_map_t(void);
  iterator begin (void) {return Base::begin();};
  iterator end (void) {return Base::end();};
  void erase (iterator pos);
  size_type erase (const Argv_t& key);
  iterator find (const Argv_t& key);
  Executable_t& operator[] (Argv_t& key);
  // insert executable if not present, replace if executable already exists
  void set(Executable_t* target); 

// map semantics
  typedef Argv_t key_type;
  typedef Base::mapped_type mapped_type;
  typedef Base::value_type value_type;
  typedef Base::key_compare key_compare;
  typedef Base::allocator_type allocator_type;
  typedef Base::reference reference;
  typedef Base::const_reference const_reference;
  typedef Base::iterator iterator;
  typedef Base::const_iterator const_iterator;
  typedef Base::size_type size_type;
  typedef Base::difference_type difference_type;
  typedef Base::reverse_iterator reverse_iterator;
  typedef Base::const_reverse_iterator const_reverse_iterator; };

extern Executable_map_t executable_map;

