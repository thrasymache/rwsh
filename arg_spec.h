// Copyright (C) 2006, 2007 Samuel Newbold

class Argv;
class Function;

enum Arg_type {FIXED, REFERENCE, SOON, STAR_REF, STAR_SOON, SELECTION,
                 SELECT_VAR, SELECT_STAR_VAR, SUBSTITUTION};

class Arg_spec {
  Arg_type type;
  unsigned soon_level;
  unsigned ref_level;
  bool expand;
  int word_selection;
  Function* substitution;
  std::string text;

  void add_function_spec(const std::string& src);
 public:
  Arg_spec(const std::string& script, unsigned max_soon);
  Arg_spec(const std::string& src, std::string::size_type style_start, 
             std::string::size_type& point, unsigned max_soon);
  Arg_spec(Arg_type type, unsigned soon_level, unsigned ref_level,
             unsigned expand_count, int word_selection,
             Function* substitution, std::string text);
  Arg_spec(const Arg_spec& src);
  ~Arg_spec();
  void apply(const Argv& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_spec> > res) const;
  void interpret(const Argv& src,
                 std::back_insert_iterator<Argv> res) const;
  void promote_soons(unsigned nesting);
  std::string str(void) const; };

