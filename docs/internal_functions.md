# internal functions
Rather than using a shell variable with its own language of special
characters to configure the prompt, rwsh uses an internal function.
Internal functions are just functions that the shell runs for you under
certain conditions. Their names all begin with "rwsh." so that they
won't conflict with other functions you might wish to define and to
clearly identify them as internal functions. Otherwise, they are normal
functions that can be run from the command line or be called by other
functions. They are divided into features and error messages, with the
difference being that if a feature function is missing, then nothing is
run, but if an error message function is missing, then
rwsh.executable\_not\_found is run instead.

## features
### rwsh.after\_command
After each command completes, rwsh.after\_command is run with the entire
command line as its arguments.

### rwsh.autofunction
If a command is requested which is not in the executable map, then the
entire command line is passed to rwsh.autofunction. Afterwards, the
command is searched for again, and if it is not found, then
rwsh.executable\_not\_found is called. Thus rwsh.autofunction must
implement whatever $PATH-searching logic is desired, and then add the
binary that it finds to the executable map as a function.

### rwsh.before\_command
Before any command is run, the entire command is passed to
rwsh.before\_command (if it exists). The command is run only if
rwsh.before\_command returns 0.<br>

### rwsh.prompt
This function is responsible for printing the prompt and for any
notification messages that should come before the prompt. It is not passed any
arguments.

### rwsh.raw\_command
This function is passed the entire command line as a single argument,
before it is parsed, and will thus have the command as typed, rather
than the command that will be run.

### rwsh.run\_logic
If it exists, this command is passed the entire command line and takes
responsibility for actually running the command. If this function fails
early enough, the command will not be run.

### rwsh.shutdown
This function is run immediately before the shell exits (i.e. after the
exit builtin is run, or after the shell receives EOF from stdin). Its
arguments are the same as those that were used to initialize the shell.
(The arguments to the rwsh executable itself, not the last set of
arguments passed to .init.)

### rwsh.vars
rwsh.vars is currently the only exception to the rule that internal
functions are run by the shell itself under the right circumstances.
&nbsp;The intent is for the configuration script to give descriptive
information about the variables used by the configured shell, since
this will probably include variables that are not used by the shell
directly, but are used instead by internal functions, or to implement
other functionality.

## error messages
Note, it any of the functions below produce an error that causes the
stack to unwind, then a generic error message will be printed for both
errors rather than calling the internal function used to report errors
on the secondary error. This prevents an infinite recursion of error
messages. Note that while ".init" is not an error message it falls into
this category because it must exist.

### rwsh.arguments\_for\_argfunction
rwsh cannot handle passing arguments to rwsh.unescaped\_argfunction,
rwsh.argfunction, and rwsh.escaped\_argfunction (e.g. "fn x {fn $1
{rwsh.argfunction $1}}" or "fn y {fn $1 {rwsh.argfunction {echo
hello}}}"), so any attempt to do so is flagged as an error, and results
in rwsh.arguments\_for\_argfunction. Currently, the only argument passed
to this function is the name of the function being called (i.e.
rwsh.argfunction, rwsh.unescaped\_argfunction, or
rwsh.escaped\_argfunction).

If you find that you want to pass arguments to an argfunction, this
is the purpose of rwsh.mapped\_argfunction. While the second example
given above would require two argument functions which is not currently
supported, the first one could be written as "fn x {fn $1
{rwsh.mapped\_argfunction $1 {rwsh.argfunction}}}"</p>

### rwsh.bad\_argc
If a command is requested with the wrong
number of arguments, then rwsh.bad\_argc is run instead.

### rwsh.bad\_argfunction\_style
the style of an argument function is determined by its prefix. The two
styles currently supported are "plain" argfunctions, which have no
prefix and are passed on to the executable, and substitutions, which
are prefixed by one or more ampersands (&), and which are run at
some point before the given executable and which are replaced with
their output. If an argfunction is specified with any other prefix, a
rwsh.bad\_argfunction\_style error will be thrown. Its one argument is
the argument function with the bad style.

### rwsh.bad\_if\_nest
if-else blocks do not need to all be at the same nesting level - you
can define functions that act like .if, .else\_if, .else, etc. with
differing semantics, which will result in a function returning after
.if was called, but before .else was called. However, if-else blocks
need to be nested within each other properly. That is to say that if
".if" occurs within the argument function of a different .if
invocation, then ".else" needs to occur within that same argument
function. If it does not, then an error will be thrown when the
argument function returns.

For example, this will generate an error:

    .if .return 0 {
        .if .return $x {.nop}}
    .else {.echo this line will never be reached}
    .else {.echo nor this}

But this will not:

    .if .return 0 {
        rwsh.mapped_argfunction {
            .if .return $x {.nop}}
        rwsh.mapped_argfunction {
            rwsh.mapped_argfunction {
                .else {.echo valid}}}}
    .else {.echo still valid}

### rwsh.binary\_not\_found
If the execve() system call fails when attempting to run a binary,
rwsh.binary\_not\_found is called with the name of the binary as its only
argument.

### rwsh.dash\_dash\_argument
In function prototypes, "[--]" is implicit, but it can be included
without causing an error. However, "[-- arg]", or "[-- foo bar]", in
which -- takes arguments is not supported, and produces this error with
the last argument in the prototype (e.g. "arg" and "bar" respectively).

### rwsh.divide\_by\_zero
If the second argument to .var\_divide is "0", or a number that converts to 0,
then this error will be thrown.

### rwsh.double\_redirection
If a command line has two output redirections, or two input
redirections, then this error is thrown. Both of these constructions
will be supported in a later version.

### rwsh.duplicate\_parameter
If a function definition lists two parameters with the same name, then
this function will be called, with the name of the parameter and a call
stack. Note that the empty string is a valid variable name, and so
placing whitespace before a close bracket, or after an open bracket is
defining an optional argument with the empty string as its name, as
does "[]".

### rwsh.else\_without\_if
If ".else" is called before ".if" or if a single ".if" has two associated
".else" statements, then this error will be thrown. 

### rwsh.excessive\_nesting
If a function nests more than $MAX\_NESTING times, execution will be
aborted and rwsh.excessive\_nesting will be called with the call stack
as arguments (the name of the last function to be called is in $1, the
function that called it is in $2, and so on). If $MAX\_NESTING is "0",
undefined or has some value other than a positive number, then
functions may be called, but they may not themselves call functions. If
$MAX\_NESTING is "1", then functions may call other functions, but those
functions may only call builtins and binaries. This is necessary
because an infinitely recursive function would cause the shell to
crash. (This behavior can be observed by setting MAX\_NESTING to an
absurdly high value, and then running a function that calls itself, or
by running a function that always calls itself in a shell like bash.)
It is also useful to have an error message in these situations that does
not fill several screens.

### rwsh.excess\_argfunction
If a builtin that cannot use an argument function is called with an argument
function, this error will be thrown. 

### rwsh.executable\_not\_found
If a function or a builtin is requested that is not in the
executable\_map after calling rwsh.autofunction, then
rwsh.executable\_not\_found is called with the same command line as was
going to be passed to the missing executable, meaning that the
executable's name is in $0.

rwsh.executable\_not\_found ends up being an incredibly important
error message because if some other error occurs, for which the
corresponding internal function has not been defined (e.g. if the error
occurred in the initialization file), then rwsh.executable\_not\_found is
called. This has two implications, the first is that it is worthwhile
to print the entire command line that is passed. While a user will
normally know the command that they just entered, if
rwsh.executable\_not\_found is being called in response to an error, then
the user will not know the arguments to that error function, which will
themselves be useful information. The second implication is that it is
unacceptable for rwsh.executable\_not\_found to not be defined if it is
called. It is not defined before .init is called, and it isn't a
crisis if it isn't initialized immediately. However, if it is called
and it is not defined, then it will be defined to a default value and
that will be run.

### rwsh.failed\_substitution
If a substitution returns nonzero, then it is understood to have
failed, and the executable for which its output was to be an argument
is not executed. Instead rwsh.failed\_substitution is run, with its
first argument being the substitution that failed.

### rwsh.file\_open\_failure
If a file input or output redirection cannot open its target, or if
.source cannot open its first argument, then this error is thrown. The
name of the file that could not be opened is passed as the first
argument.

### rwsh.if\_before\_else
If two ".if" commands occur, without a ".else" command between them, then this
error is thrown.

### .init
This function is responsible for all of the initialization of the shell. The
command line that started rwsh is passed to .init as its arguments. It doesn't
do any good to change .init after the shell has started, but it can be examined
to see what initialization has been done, and it can be run as a command to
reinitialize the shell, possibly with different arguments. This is why it is
named like a built-in rather than like an internal function.

### rwsh.input\_range
If any of the builtins that take a numerical argument are passed a
number outside of the range of a double-precision floating point
number, then this error will be thrown.

### rwsh.invalid\_word\_selection
If a word selection (i.e. something following a dollar sign following a
variable name) is not a number, this error will be thrown. Its argument
will be the text that was the word selection.

### rwsh.mismatched\_brace
If the braces in a command do not match properly, (implying a confusion
as to where an argument function begins and ends), then
rwsh.mismatched\_brace is called instead. It's one argument is the
command string up to and including the first unmatched brace.

### rwsh.mismatched\_bracket
If the brackets in the definition of a function taking options do not
match properly, then rwsh.mismatched\_bracket is called instead. It's
first argument is the section of the command string with the
mismatched brakcet, up to and including the first unmatched bracket,
and then it also takes a call stack.

### rwsh.mismatched\_parenthesis
If a parenthesis is found for which no mate can be found, this error
will be thrown with the offending text passed as an argument. Note that
this will generally be an excess close parenthesis since in the
presence of an excess open parenthesis the parser will continue taking
more text until a mate is found.

### rwsh.multiple\_argfunctions
rwsh cannot currently handle more than one argfunction per
command. This does not limit the ability of argfunctions to be
nested within other argfunctions, or even several argfunctions to be
nested within a single argfunction as long as each is associated with a
separate command. If a command is requested with multiple argfunctions,
then rwsh.multiple\_argfunctions is called instead. It does not take any
arguments.

### rwsh.not\_a\_number
If any builtin that expects a number as an argument (such as .var\_add,
.test\_less, or .return) gets instead something that is not simply
numerical (e.g. "many", "three", or "3a"), this error is thrown with
the text that was where a number should have been passed as an argument.

### rwsh.not\_executable
If a script is sourced that does not have the executable permission bit
set, this error is thrown and the name of the script is passed as an
argument.

### rwsh.not\_soon\_enough
it is possible to specify a soon variable or a substitution that is
delayed past the execution of the statement in which it occurs (e.g.
"echo &&X" or "rwsh.mapped\_argfunction {echo &&&X}"). If a command is
given in which this will be the case, an rwsh.not\_soon\_enough error is
thrown, it's argument is the argument specification that is delayed too far.

### rwsh.result\_range
If the result of an arithmetic operation does not fit in a double
precision number on the machine, this error is thrown with the two
components of the arithmetic operation passed as arguments

### rwsh.selection\_not\_found
If a selection read fails to return any matches, then
rwsh.selection\_not\_found is called, with the smallest part of the
selection that did not find a match as its first argument and the full
selection as its second argument. For example, a command line that read
"/bin/ls @/usrr/local/" would result in a call of
"rwsh.selection\_not\_found /usrr /usrr/local/\*"

<h3>rwsh.sighup rwsh.sigint rwsh.sigquit rwsh.sigpipe rwsh.sigterm
rwsh.sigtstp rwsh.sigcont rwsh.sigchld rwsh.sigusr1 rwsh.sigusr2
rwsh.sigunknown</h3>
If the shell receives any of these signals, then it will unwind the
stack, and pass the call stack as the arguments to the corresponding
internal function.

### rwsh.undefined\_variable
If an argument specification reads a variable that is not set, or
builtins such as .selection\_set or .var\_add are passed a variable that
is not set, then an rwsh.undefined\_variable error is thown. Its argument
is the name of the variable.

### rwsh.unreadable\_dir
If a selection read encounters a directory to which you do not have
read permissions, then rwsh.unreadable\_dir is called with the name of
that directory as its argument.

### rwsh.unrecognized\_flag
If a flag argument (i.e. one that begins with a dash that does not
follow "--") is passed to a function that declares all of its flags
(such as functions defined with .function\_all\_options), that is not one
of the declared flags, then rwsh.unrecognized\_flag is run with the
offending flag and a call stack as its arguments.

### rwsh.unchecked\_variable
If an optional parameter to a function was not passed and the function ended
without having any code depend upon the the existence of this variable, then
this exception is thrown with the name of the variable. The expectation is that
the variable was misnamed either when it was declared, when it was checked, or
there is some essential code that was not written or not executed. (If the
optional parameter was passed, then an rwsh.unused\_variable exception would be
thrown instead.) See the section on unused and unchecked variables in the
prototypes page for more information.

### rwsh.unused\_variable
If a block is exited without all of the variables that are defined only in that
block being used, then this exception is thrown with the name of the variable.
This will generally be a local variable or a parameter to a function. The
expectation is that the variable was misnamed either when it was declared, when
it was used, or that the function ended before some essential part was run.
Again see the section on unused and unchecked variables in the prototypes page
for more information.

### rwsh.version\_incompatible
If .version\_compatible is run with the version string different from
the set of versions that this shell is compatible with, then this error
will be thrown with the requested version as an argument. At the
current time there are so many changes between one release and the next
that this will occur if the version string passed is not identical to
that printed by .version. At some point in the future (likely after
1.0), the shell will be backwards compatible for scripts that do not
depend upon bugs or the absence of features, for some range of
versions. At that time this error will only be thrown if the version is
outside of that range of compatible versions.
