// Copyright (C) 2006, 2007 Samuel Newbold

class Argv_t;
class Function_t;

enum Arg_type_t {FIXED, REFERENCE, SOON, STAR_REF, STAR_SOON, SELECTION,
                 SELECT_VAR, SELECT_STAR_VAR, SUBSTITUTION};

class Arg_spec_t {
  Arg_type_t type;
  unsigned soon_level;
  unsigned ref_level;
  unsigned expand_count;
  Function_t* substitution;
  std::string text;

  void add_function_spec(const std::string& src);
 public:
  Arg_spec_t(const std::string& script, unsigned max_soon);
  Arg_spec_t(const std::string& src, std::string::size_type style_start, 
             std::string::size_type& point, unsigned max_soon);
  Arg_spec_t(Arg_type_t type, unsigned soon_level, unsigned ref_level,
             unsigned expand_count, Function_t* substitution, std::string text);
  Arg_spec_t(const Arg_spec_t& src);
  ~Arg_spec_t();
  void apply(const Argv_t& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_spec_t> > res) const;
  void interpret(const Argv_t& src,
                 std::back_insert_iterator<Argv_t> res) const;
  void promote_soons(unsigned nesting);
  std::string str(void) const; };

