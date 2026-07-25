// Copyright (C) 2005-2026 Samuel Newbold

typedef std::vector<std::string> Argv;
struct Error_list;

namespace E {
  enum Exception_t {
    No_exception,
    Ambiguous_prototype_dash_dash,
    Arguments_for_argfunction,
    Autofunction,
    Bad_argfunction_style,
    Bad_args,
    Bad_if_nest,
    Binary_does_not_exist,
    Binary_not_found,
    Break,
    Continue,
    Dash_dash_argument,
    Dash_star_argument,
    Directory_not_found,
    Divide_by_zero,
    Double_redirection,
    Duplicate_parameter,
    Elipsis_first_arg,
    Else_without_if,
    Epsilon,
    Excess_argfunction,
    Excessive_exceptions_collected,
    Excessive_exceptions_in_catch,
    Excessive_nesting,
    Exec_failed,
    Executable_already_exists,
    Failed_substitution,
    False,
    File_open_failure,
    File_not_found,
    Fixed_argument,
    Flag_in_elipsis,
    Function_not_found,
    Global_would_be_masked,
    If_before_else,
    Illegal_function_name,
    Illegal_variable_name,
    Input_range,
    Internal_error,
    Interrupted_sleep,
    Invalid_word_selection,
    // Line_continuation,
    Mismatched_brace,
    Mismatched_bracket,
    Mismatched_parenthesis,
    Missing_argfunction,
    Multiple_argfunctions,
    Not_a_directory,
    Not_a_function,
    Not_a_number,
    Not_catching_exception,
    Not_executable,
    Not_soon_enough,
    Number_not_an_integer,
    Raw_command,
    Return_code,
    Result_range,
    Post_dash_dash_flag,
    Prompt,
    Selection_not_found,
    Shutdown,
    Sighup,
    Sigint,
    Sigquit,
    Sigpipe,
    Sigterm,
    Sigtstp,
    Sigcont,
    Sigchld,
    Sigusr1,
    Sigusr2,
    Sigunknown,
    Tardy_flag,
    Unchecked_variable,
    Unclosed_brace,
    Unclosed_parenthesis,
    Undeclared_variable,
    Undefined_variable,
    Unfinished_if_block,
    Unreadable_dir,
    Unrecognized_flag,
    Unused_before_set,
    Unused_variable,
    Variable_already_exists,
    Version_incompatible,
    Exception_count};}

extern std::string exception_names[E::Exception_count];

struct Exception : public Argv {
  E::Exception_t exception;
  Exception(E::Exception_t exception);
  Exception(E::Exception_t exception, const std::string& value);
  Exception(E::Exception_t exception, const std::string& value, int errno_v);
  Exception(E::Exception_t exception, const std::string& x, const std::string& y);
  Exception(E::Exception_t exception, const std::string& w, const std::string& x,
              const std::string& y, const std::string& z);
  Exception(E::Exception_t exception, int x);
  Exception(E::Exception_t exception, int x, int y);
  Exception(E::Exception_t exception, int x, int y, int z); };

struct Undefined_variable : public Exception {
  Undefined_variable(const std::string& name_i) :
    Exception(E::Undefined_variable, name_i) {}};

class Old_argv {
  char** focus;
  int argc_v;

 public:
  Old_argv(const Argv& src);
  ~Old_argv(void);
  char** const argv(void) const {return focus;};
  int argc(void) {return argc_v;}; };
