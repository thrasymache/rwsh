// the registering of builtins
// Copyright Samuel Newbold 2005-2019

#include <list>
#include <map>
#include <set>
#include <signal.h>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "arg_script.h"
#include "argm.h"
#include "rwsh_init.h"
#include "builtin.h"
#include "call_stack.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

void bi(const std::string& name,
        void (*implementation)(const Argm& argm, Error_list& exceptions),
        const Argv& parameters) {
    executable_map.set(new Builtin(name, implementation, parameters));}

void fn(const std::string& name, const Argv& parameters,
        const std::string& src, Error_list& exceptions) {
    std::string::size_type point = 0;
    Command_block body(src, point, 0, exceptions);
    if (point != src.length() && point != std::string::npos) std::abort();
    // testing this error handling requires bad functions in internal_init()
    else if (global_stack.unwind_stack())
      global_stack.exception_handler(exceptions);
    else executable_map.set(new Function(name, parameters, body));}

void internal_init(Error_list& exceptions) {
  bi(".argc", b_argc, Argv {"--", "[list", "...]"});
  bi(".binary", b_binary, Argv {"file"});
  bi(".cd", b_cd, Argv {"--", "path"});
  bi(".collect_errors_except", b_collect_errors_except,
     Argv {"--", "exceptions", "...", ".{argfunction}"});
  bi(".collect_errors_only", b_collect_errors_only,
     Argv {"--", "exceptions", "...", ".{argfunction}"});
  bi(".combine", b_combine, Argv {"--", "text", "..."});
  bi(".enable_readline", b_enable_readline, Argv {});
  bi(".echo", b_echo, Argv {"--", "text", "..."});
  bi(".error", b_error, Argv {"--", "text", "..."});
  bi(".disable_readline", b_disable_readline, Argv {});
  bi(".else", b_else, Argv {".{argfunction}"});
  bi(".else_if", b_else_if, Argv{"--", "condition", "...", ".{argfunction}"});
  bi(".else_if_not", b_else_if_not,
     Argv{"--", "condition", "...", ".{argfunction}"});
  bi(".exec", b_exec, Argv{"--", "command", "..."});
  bi(".execution_count", b_execution_count, Argv {"--", "command"});
  bi(".exit", b_exit, Argv{"--", "return_code"});
  bi(".fallback_handler", b_fallback_handler,
     Argv {"--", "command", "...", "[.{argfunction}]"});
  bi(".get_fallback_message", b_get_fallback_message, Argv {});
  bi(".get_max_collectible_exceptions", b_get_max_collectible_exceptions,
     Argv {});
  bi(".get_max_extra_exceptions", b_get_max_extra_exceptions, Argv {});
  bi(".get_max_nesting", b_get_max_nesting, Argv {});
  bi(".getpid", b_getpid, Argv {});
  bi(".getppid", b_getppid, Argv {});
  bi(".for", b_for, Argv {"--", "list", "...", ".{argfunction}"});
  bi(".for_each_line", b_for_each_line, Argv {".{argfunction}"});
  bi(".fork", b_fork, Argv {"--", "command", "...", "[.{argfunction}]"});
  bi(".function", b_function, Argv {"--", "name", ".{argfunction}"});
  bi(".function_all_flags", b_function_all_flags,
     Argv {"--", "name", "[prototype", "...]", ".{argfunction}"});
  bi(".global", b_global, Argv {"--", "var", "value"});
  bi(".if", b_if, Argv {"--", "condition", "...", ".{argfunction}"});
  fn(".init",  Argv {"--", "[args", "...]"},
      ".set_max_nesting 10\n"
      "    .function_all_flags .file_open_failure name stack ... {.nop\n"
      "        .combine (init file ) $name ( does not exist\n"
      "call stack ) $stack (\n)}\n"
      "    .function_all_flags .raw_command -- args ... {.nop $args}\n"
      "    .collect_errors_except .nop {.nop\n"
      "      .source /etc/rwshrc $args$\n"
      "      .for &{.internal_functions}$ {.nop\n"
      "        .if .test_executable_exists $1 {.nop}\n"
      "        .else {.echo &&&1 not defined (\n)}}\n"
      "      .if .test_executable_exists .help {.nop\n"
      "        .if .test_not_empty ${.help} {.nop}\n"
      "        .else {.echo .help produces no output (\n)}}\n"
      "      .else {.echo .help not defined (\n)}}", exceptions);
  fn(".internal_features", Argv {},
     ".echo (.after_command .before_command .run_logic\n)", exceptions);
  bi(".internal_functions", b_internal_functions, Argv {});
  fn(".internal_vars", Argv {}, ".echo (FIGNORE\n)", exceptions);
  bi(".is_default_input", b_is_default_input, Argv {});
  bi(".is_default_output", b_is_default_output, Argv {});
  bi(".is_default_error", b_is_default_error, Argv {});
  bi(".last_exception", b_last_exception, Argv{"--", "command"});
  bi(".last_execution_time", b_last_execution_time, Argv{"--", "command"});
  bi(".list_environment", b_list_environment, Argv {});
  bi(".list_executables", b_list_executables, Argv {});
  bi(".list_locals", b_list_locals, Argv {});
  bi(".local", b_local, Argv {"--", "var", "value"});
  bi(".local_declare", b_local_declare, Argv {"--", "var", "..."});
  bi(".ls", b_ls, Argv {"--", "paths", "..."});
  bi(".nop", b_nop, Argv {"--", "[args", "...]", "[.{argfunction}]"});
  bi(".replace_exception", b_replace_exception,
     Argv {"--", "args", "...", "[.{argfunction}]"});
  bi(".rm_executable", b_rm_executable, Argv {"--", "command"});
  bi(".scope", b_scope,
     Argv {"--", "[list", "...]", "prototype", ".{argfunction}"});
  bi(".selection_set", b_selection_set, Argv {"--", "var", "value", "..."});
  bi(".set", b_set, Argv {"--", "var", "value", "..."});
  bi(".set_fallback_message", b_set_fallback_message, Argv {"message", "..."});
  bi(".set_max_collectible_exceptions", b_set_max_collectible_exceptions,
     Argv {"--", "maximum"});
  bi(".set_max_extra_exceptions", b_set_max_extra_exceptions,
     Argv {"--", "maximum"});
  bi(".set_max_nesting", b_set_max_nesting, Argv {"--", "maximum"});
  fn(".shutdown", Argv {"--", "[args", "...]"}, ".nop $args; .exit 10",
     exceptions);
  bi(".source", b_source, Argv {"--", "file", "[args", "...]"});
  bi(".stepwise", b_stepwise, Argv {"--", "command", "...", ".{argfunction}"});
  bi(".store_output", b_store_output, Argv {"--", "var", ".{argfunction}"});
  bi(".test_executable_exists", b_test_executable_exists,
     Argv {"--", "command", "[.{argfunction}]"});
  bi(".test_file_exists", b_test_file_exists, Argv {"--", "path", "..."});
  bi(".test_greater", b_test_greater, Argv {"--", "lhs", "rhs"});
  bi(".test_is_number", b_test_is_number, Argv {"--", "value"});
  bi(".test_in", b_test_in, Argv {"--", "focus", "[list", "...]"});
  bi(".test_less", b_test_less, Argv {"--", "lhs", "rhs"});
  bi(".test_not_empty", b_test_not_empty, Argv {"--", "[input", "...]"});
  bi(".test_number_equal", b_test_number_equal, Argv {"--", "lhs", "rhs"});
  bi(".test_string_equal", b_test_string_equal,
     Argv {"--", "left_string", "right_string"});
  bi(".test_string_unequal", b_test_string_unequal,
     Argv {"--", "left_string", "right_string"});
  bi(".throw", b_throw, Argv {"--", "exception", "...", "[.{argfunction}]"});
  bi(".toggle_readline", b_toggle_readline, Argv {});
  bi(".total_execution_time", b_total_execution_time, Argv{"--", "command"});
  bi(".try_catch_recursive", b_try_catch_recursive,
     Argv {"--", "exception", "...", ".{argfunction}"});
  bi(".type", b_type, Argv {"--", "command", "[.{argfunction}]"});
  bi(".unset", b_unset, Argv {"--", "var"});
  bi(".usleep", b_usleep, Argv {"--", "milliseconds"});
  bi(".usleep_overhead", b_usleep_overhead, Argv {});
  bi(".waiting_for_binary", b_waiting_for_binary, Argv {});
  bi(".waiting_for_shell", b_waiting_for_shell, Argv {});
  bi(".waiting_for_user", b_waiting_for_user, Argv {});
  bi(".whence_function", b_whence_function,
     Argv {"--", "command", "[.{argfunction}]"});
  bi(".which_path", b_which_path, Argv {"--", "command", "PATH"});
  bi(".while", b_while, Argv {"--", "command", "...", ".{argfunction}"});
  bi(".var_add", b_var_add, Argv {"--", "var", "value"});
  bi(".var_divide", b_var_divide, Argv {"--", "var", "value"});
  bi(".var_exists", b_var_exists, Argv {"--", "var", "..."});
  bi(".var_multiply", b_var_multiply, Argv {"--", "var", "value"});
  bi(".var_subtract", b_var_subtract, Argv {"--", "var", "value"});
  bi(".version", b_version, Argv {});
  bi(".version_compatible", b_version_compatible,
     Argv {"--", "candidate_version"});}

inline Argm::Exception_t unix2rwsh(int sig) {
  switch (sig) {
    case SIGHUP: return Argm::Sighup;
    case SIGINT: return Argm::Sigint;
    case SIGQUIT: return Argm::Sigquit;
    case SIGPIPE: return Argm::Sigpipe;
    case SIGTERM: return Argm::Sigterm;
    case SIGTSTP: return Argm::Sigtstp;
    case SIGCONT: return Argm::Sigcont;
    case SIGCHLD: return Argm::Sigchld;
    case SIGUSR1: return Argm::Sigusr1;
    case SIGUSR2: return Argm::Sigusr2;
    default: return Argm::Sigunknown;}}

void unix_signal_handler(int sig) {
  global_stack.caught_signal = unix2rwsh(sig);}

void register_signals(void) {
  signal(SIGHUP, unix_signal_handler);
  signal(SIGINT, unix_signal_handler);
  signal(SIGQUIT, unix_signal_handler);
  signal(SIGPIPE, unix_signal_handler);
  signal(SIGTERM, unix_signal_handler);
  signal(SIGTSTP, unix_signal_handler);
  signal(SIGUSR1, unix_signal_handler);
  signal(SIGUSR2, unix_signal_handler);}
