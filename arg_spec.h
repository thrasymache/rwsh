// Copyright (C) 2006-2015 Samuel Newbold

class Argm;
class Command_block;

enum Arg_type {FIXED, REFERENCE, SOON, STAR_REF, STAR_SOON, SELECTION,
                 SELECT_VAR, SELECT_STAR_VAR, SOON_SUBSTITUTION, SUBSTITUTION};

class Arg_spec {
  Arg_type type;
  unsigned soon_level;
  unsigned ref_level;
  bool expand;
  int word_selection;
  Command_block* substitution;
  std::string text;
  std::string trailing;

  void add_function_spec(const std::string& src);
 public:
  Arg_spec(const std::string& script, unsigned max_soon);
  Arg_spec(const std::string& src, std::string::size_type style_start, 
           std::string::size_type& point, unsigned max_soon);
  Arg_spec(Arg_type type, unsigned soon_level, unsigned ref_level,
           unsigned expand_count, int word_selection,
           Command_block* substitution, const std::string& text,
           const std::string& trailing);
  Arg_spec(const Arg_spec& src);
  ~Arg_spec();
  void apply(const Argm& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_spec> > res) const;
  void interpret(const Argm& src,
                 std::back_insert_iterator<Argm> res) const;
  void promote_soons(unsigned nesting);
  std::string str(void) const; };

