// Copyright (C) 2006-2017 Samuel Newbold

class Argm;
class Command_block;
class Error_list;

enum Arg_type {FIXED, REFERENCE, SOON, STAR_REF, STAR_SOON, SELECTION,
                 SELECT_VAR, SELECT_STAR_VAR, SOON_SUBSTITUTION, SUBSTITUTION};

class Arg_spec {
  Arg_type type;
  unsigned soon_level;
  unsigned ref_level;
  unsigned expand_count;
  int word_selection;
  Command_block* substitution;
  std::string text;
  std::string trailing;

  template<class Out> Out evaluate_expansion(const std::string& value,
                                             Out res) const;
  template<class Out> Out evaluate_substitution(const Argm& src, Out res,
                                             Error_list& exceptions) const;
  template<class Out> Out evaluate_var(const Argm& src, Out res) const;
  void parse_brace_type(const std::string& src, unsigned max_soon,
                   std::string::size_type style, std::string::size_type end,
                   Error_list& errors);
  void parse_word_selection(const std::string& src,
                          std::string::size_type& point, Error_list& errors);
 public:
  Arg_spec(const std::string& script, unsigned max_soon, Error_list& errors);
  Arg_spec(const std::string& src, std::string::size_type style,
         std::string::size_type& point, unsigned max_soon, Error_list& errors);
  Arg_spec(Arg_type type, unsigned soon_level, unsigned ref_level,
           unsigned expand_count, int word_selection,
           Command_block* substitution, const std::string& text,
           const std::string& trailing);
  Arg_spec(const Arg_spec& src);
  Arg_spec(const std::string& src);
  ~Arg_spec();
  void apply(const Argm& src, unsigned nesting,
             std::back_insert_iterator<std::vector<Arg_spec> > res,
             Error_list& exceptions) const;
  void interpret(const Argm& src,
                 std::back_insert_iterator<Argm> res,
                 Error_list& exceptions) const;
  void promote_soons(unsigned nesting);
  std::string str(void) const; };

