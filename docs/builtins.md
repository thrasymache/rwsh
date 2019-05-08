Builtins take a fixed number of arguments, and will throw an argument count
error if they are passed a different number. Builtins either return nonzero or
throw an error in the case of failures, rather than printing any diagnostic
text. They are not intended to regularly be useful as something to run from the
command line. Instead they are intended to be useful building blocks for shell
functions.
**** .cd ****
The .cd changes the current working directory to the path given as its
argument. If successful it sets CWD to the argument that was passed to it. It
returns the value returned by chdir.
    .cd does not open any file descriptors
**** .combine ****
The .combine command prints its arguments without any intervening space. It
does not append a newline. Note that this is the way to combine the contents of
multiple variables or to combine the contents of a variable with fixed text in
a single word.
    .combine opens the output file descriptor.
**** .echo ****
The .echo command prints its arguments separated by a single space. It does not
append a newline. An echo function that prints its arguments followed by a
newline can easily be created from the .echo builtin and the .newline builtin.
    .echo opens the output file descriptor.
**** .else ****
.else does not take any arguments, it runs its argfunction if IF_TEST is false,
and then sets IF_TEST to the empty string. It is a syntax error for .else to
occur except following .if and zero or more .else_if and .else_if_not commands,
or for a sequence of those commands to occur without a .else.
    .else does not itself open any file descriptors.
**** .else_if ****
.else_if returns 0 immediately if IF_TEST is true. Otherwise it runs its
arguments, setting IF_TEST to true, and running any argfunction if they return
0. It is a syntax error for .else_if to occur except between .if and .else.
    .else_if does not itself open any file descriptors.
**** .else_if_not ****
.else_if_not is identical to .else_if except that it sets IF_TEST to true and
runs its argfunction only if the result of running its arguments returns
nonzero.
    .else_if_not does not itself open any file descriptors.
**** .exec ****
.exec attempts to replace the shell with another executable using the exec
system call. The first argument is the name of the executable, any remaining
arguments are passed to that executable. The variable map is used as the new
environment. The current input, output and error file descriptors are the
default input, output, and error for the executable. .exec can only return non-
zero since it will never return on success.
    .exec does not open any file descriptors.
**** .exit ****
.exit causes the shell to exit before the next command is requested from the
user. It does not terminate any function (i.e. commands after .exit in a
function will be run). After any enclosing functions complete, rwsh.shutdown
will be run before the shell actually terminates.
    .exit does not open any file descriptors.
**** .for ****
.for runs its argfunction once for each of its arguments, passing the argument
to the argfunction each time as $1.
    .for does not open any file descriptors.
**** .for_each_line ****
.for_each_line runs its argfunction once for each line of its input, passing
the line as arguments to its argfunction.
    .for_each_line opens its input file descriptor.
**** .fork ****
**** .function ****
.function inserts its argument function into the executable map. The first
argument is used as the name for the new function.
    .function does not open any file descriptors.
**** .global ****
.global takes two arguments, a variable name and a value. If the variable does
not exist it is added to the variable_map with the given value, if the variable
does exist, then .global returns 1 and does not change the variable map.
Variables created in this way remain set after the termination of the enclosing
function.
    .global does not open any file descriptors.
**** .if ****
.if executes its arguments as a command, and then runs its argument function if
it has one and if the command returned 0. It is a syntax error for .if to
follow another .if, .else_if, or .else_if_not, unless there is an .else between
them. It is not a syntax error for .if to have no argfunction. .if returns -
1 on a syntax error, the return value of its argfunction if it is run, and 0
otherwise. It sets IF_TEST to true if its arguments returned 0, and to false
otherwise.
    .if does not itself open any file descriptors.
**** .importenv_overwrite ****
.importenv causes the environment that rwsh was run in to be imported into the
variable map. If a variable exists in both environments, the external value is
taken.
    .importenv_overwrite does not open any file descriptors.
**** .importenv_preserve ****
.importenv causes the environment that rwsh was run in to be imported into the
variable map. If a variable exists in both environments, the internal value is
taken.
    .importenv_preserve does not open any file descriptors.
**** .internal_errors ****
.internal_errors takes no arguments. It prints a list of those internal
functions which are run whether they exist or not (and will thus result in
rwsh.executable_not_found errors if they do not exist). Except for .init they
are all reporting errors themselves.
    .internal_errors is implemented as a function which calls .echo, which
opens  its output.
**** .internal_features ****
.internal_features takes no arguments. It prints a list of those internal
functions which are run only if they exist.
    .internal_features is implemented as a function which calls .echo, which
opens its output.
**** .internal_vars ****
.internal_vars takes no arguments. It prints a list of those variables that are
written by the shell itself or affect the running of builtins.
    .internal_vars is implemented as a function which calls .echo, which opens
its output.
**** .is_default_input ****
returns 1 if its input has been redirected away from the stdin of the shell.
    .is_default_input does not open any file descriptors.
**** .is_default_output ****
returns 1 if its output has been redirected away from the stdout of the shell.
    .is_default_output does not open any file descriptors.
**** .is_default_error ****
returns 1 if its error has been redirected away from the stdout of the shell.
    .is_default_error does not open any file descriptors.
**** .ls ****
As it currently stands, .ls is a very stripped-down version of the standard ls.
Selection write and read work just as well with external commands as with
built-ins, so feel free to define the function ls to use /bin/ls rather than
.ls.
    .ls opens its output.
**** .nop ****
.nop ignores its arguments, and leaves $? unchanged
    .nop does not open any file descriptors.
**** .return ****
.return takes one argument, which it converts to an integer, and then returns
that value. It has more error conditions than any other builtin, besides
.var_add and .var_divide.
    .return does not open any file descriptors.
**** .set ****
.set changes the value of a variable in the variable map. the first argument is
the name of the variable, its second and any additional arguments are the value
of the variable.
    .set does not open any file descriptors.
**** .selection_set ****
.selection_set is described in the selection document. Briefly, it is a variant
of .set, that treats the current value of the variable as a path that should be
modified by the second argument.
    .selection_set does not open any file descriptors.
**** .source ****
.source attempts to run its first argument as a shell script. The remaining
arguments are passed to that shell script as its arguments. .source returns the
value returned by the script it ran.
    .source does not open any file descriptors (the script gets opened as a new
file descriptor)
**** .stepwise ****
.stepwise treats its arguments as a function call, looks up this function and
interprets it with the given arguments, but rather than running this function,
it passes each statement in turn to its argument function.  To see what this
means in practice look at test.result, where stepwise commands are listed along
with their output. Also look at the functions "or" and "and" in rwshrc. The or
function runs another function, stopping after the first statement that
succeeds.  The and function runs another function, stopping after the first
statement that fails.  This is equivalent to rewriting a function in another
shell replacing each ";" with "||", and "&&" respectively.
    .stepwise does not itself open any file descriptors.
**** .store_output ****
.store_output takes one argument and one argfunction. It runs the argfunction
and if the argfunction returns zero, it stores the output in the variable named
by its argument (which must exist in advance.) .store_output returns the value
returned by the argfunction. This is the way to handle cases where you want a
substitution, but also want specific handling of any error.
    .store_output does not itself open any file descriptors.
**** .test_equal ****
.test_equal takes two arguments. It returns 0 if those two arguments are the
same string. It returns 1 if those two arguments are different strings.
    .test_equal does not open any file descriptors.
**** .test_greater ****
.test_greater takes two arguments. It returns 0 if both arguments convert to
numbers and the first argument is numerically greater than the second argument.
It returns 1 if the first argument is numerically less than or equal to the
second argument.
    .test_greater does not open any file descriptors.
**** .test_less ****
.test_less takes two arguments. It returns 0 if both arguments convert to
numbers and the first argument is numerically less than the second argument. It
returns 1 if the first argument is numerically greater than or equal to the
second argument.
    .test_less does not open any file descriptors.
**** .test_is_number ****
.test_is_number takes one argument. It returns 0 if that argument can convert
to a floating point number.
    .test_is_number does not open any file descriptors.
**** .test_not_empty ****
.test_not_empty takes one argument. It returns 0 if that argument is not the
empty string.
    .test_not_empty does not open any file descriptors.
**** .test_not_equal ****
.test_not_equal takes two arguments. It returns 0 if those two arguments are
different strings. It returns 1 if those two arguments are the same string.
    .test_not_equal does not open any file descriptors.
**** .test_number_equal ****
.test_number_equal takes two arguments. It returns 0 if both of those arguments
convert to the same floating point number. It returns 1 if those two arguments
convert to different floating point numbers. It throws an error if it cannot
convert either argument to a double presion float.
    .test_number_equal does not open any file descriptors.
**** .unset ****
.unset takes one argument, the name of a variable to remove from the variable
map.
    .unset does not open any file descriptors.
**** .usleep ****
.usleep takes one argument and then sleeps the shell for the specified number
of microseconds.
    .usleep does not open any file descriptors.
**** .var_add ****
.var_add takes two arguments, the name of a variable and an integer value to
add to this variable.  The set of errors is argument_count, not_a_number,
input_range, and result_range.
    .var_add does not open any file descriptors.
**** .var_subtract ****
.var_subtract takes two arguments, the name of a variable and an integer value
to subtract from this variable. The set of errors is argument_count,
not_a_number, input_range, and result_range.
    .var_divide does not open any file descriptors.
**** .var_divide ****
.var_divide takes two arguments, the name of a variable and an integer value
to divide this variable by. The set of errors is argument_count, not_a_number,
input_range, result_range, and divide_by_zero.
    .var_divide does not open any file descriptors.
**** .var_exists ****
.var_exists takes one argument, the name of a variable in the variable map. If
the variable is set then it returns 0. If the variable is not set it returns 1.
    .var_exists does not open any file descriptors.
**** .version ****
.version prints out the version of rwsh that is currently running
    .version opens its output.
**** .version_available ****
.version_available takes no arguments, and prints out a list of those that
would return true if passed to .version_compatible.
    .version_available opens its output.
**** .version_compatible ****
.version_compatible takes one argument, and returns 0 if that argument is an
rwsh version number compatible with this one. At the current time, only the
current version will cause .version_compatible to return 0.
    .version_compatible does not open any file descriptors.
**** .waiting_for_binary ****
.waiting_for_binary prints the total amount of time that the shell has spent
waiting for other processes that it has forked to terminate.
    .waiting_for_binary opens its output.
**** .waiting_for_shell ****
.waiting_for_shell prints the amount of time since startup that the shell has
not been waiting for input from the user, or waiting for a binary to terminate,
so that the sum of the three will exactly equal the wall time elapsed since
startup.
    Although it probably doesn't matter, all of the timing calculations are
included within .waiting_for_shell.
    .waiting_for_shell opens its output.
**** .waiting_for_user ****
.waiting_for_user prints the total amount of time that the shell has spent
waiting for commands to be entered.
    .waiting_for_user opens its output.
**** .which_executable ****
.which_executable prints the value stored in the executable map at the name
given in its first argument. Functions are printed by giving the command that
would create that function. Built-ins just print their name. Executable files
are added to the executable map when they are run. If an executable file has
been run, then .which_executable will print the name of the file, otherwise it
will inform you that it is not in the executable map, whether or not the file
exists and is executable. rwsh.mapped_argfunction and rwsh.argfunction are both
recognized as names for whatever argument function might have been passed to
.which_executable, and are printed with just their enclosing braces.
    .which_executable opens its output.

**** .which_execution_count ****
.which_execution_count prints the number of times that the executable specified
by its first argument has been run. It returns 1 if the executable does not
exist in the executable map.
    .which_execution_count opens its output.

**** .which_last_execution_time ****
.which_last_execution_time prints the amount of time that the executable
specified by its first argument took to run the last time it was run. This is
wall time as opposed to time that the shell was active. It returns 1 if the
executable does not exist in the executable map.
    .which_last_execution_time opens its output.

**** .which_total_execution_time ****
.which_total_execution_time prints the amount of time that the executable
specified by its first argument has taken to run since it was added to the
executable map. It returns 1 if the executable does not exist in the executable
map.
    .which_total_execution_time opens its output.
**** .which_path ****
.which_path takes two arguments. The first is the name of the executable to
search for, the second is the colon-separated search path within which to look
for it. If an executable is found, it is printed, otherwise an error is
returned.
    .which_path opens its output.
**** .which_return ****
.which_return takes one argument, the name of an executable in the executable
map. If the executable is found, then it prints the last value returned by that
executable, and then returns 0. If the executable is not found, then it returns
1. If the executable name is rwsh.argfunction or rwsh.mapped_argfunction, then
it returns 2.
    .which_return opens its output.
**** .which_test ****
.which_test takes one argument, the name of an executable to look up in the
executable map. If one is found it returns 0. Otherwise it returns 1. It does
not print any text.
    .which_test does not open any file descriptors.
**** .while ****
.while runs its arguments as a command, and for each time that they return 0,
it runs its argument function.  Note: a loop of the form ".while .return $X
{...}" will either not run its argument function or run it until it throws an
error.  $X is interpretted once, before the statement is run, and however the
argument function might change the value of X, it cannot change the value that
it used to have.
    .while does not itself open any file descriptors.
