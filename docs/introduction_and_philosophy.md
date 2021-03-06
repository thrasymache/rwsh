# introduction and philosophy

### philosophy

The first concrete idea that I had for the interface of rwsh, was that
I wanted a shell that knew that after you'd done something with a few
files, more than likely you'd want to do something else with a similar
group of files. I also wanted it to be able to tell the difference
between the regular expression I was grepping for, and the pattern that
described the files I was looking in. A program can't be omniscient, so
this was going to require a lot of configuration. In working out how
that configuration might occur I remembered that the only good reason
for anybody to learn all of the arcane trivia required to use a shell
is how much more expressive a language can be than a set of toggles.
Out of this grew a philosophy that the shell shouldn't do anything much
more than provide you with a language to describe how you wanted it to
act. So, at startup, the only variables set and only functions in the
executable\_map are those that are reasonably necessary to load a
configuration script that will set things properly. No parsing is done
of arguments before they are
passed to .init, and there is no expectation that if the shell
cannot find a startup script that it will be useful for anything except
loading a sane configuration.

### exploring .init

Somehow commenting on what happens between when rwsh starts executing
and when it starts accepting input seems like the best way to introduce
it. This serves the double purpose of showing its style of
doing
things, and explaining how to reconfigure it.

If you run ".which\_executable .init", you will get the following
output.


    .function .init {.set MAX_NESTING 4; .source /etc/rwshrc $*}

The only configuration and initialization that happens before .init
is called is to add builtin commands to the executable\_map, to
reformat the arguments to rwsh into something that can be accepted by a
shell function, and to initialize .init itself. In rwsh,
all
builtins begin with ".", and the only way to run a separate executable
is
to specify its absolute path. Everything else is a shell
function. Shell functions that begin with "rwsh." are
referred to
as internal functions, they are usually run by the shell itself under
certain circumstances. Internal functions are like other functions in
every other way. In particular any of them can be run from the command
line at any time. .init is the internal function that is
run
before stdin is opened and the first command is accepted. The
first thing it must do is set MAX\_NESTING to something sane so that it
can run .source. Something of an explanation for why is
given in the internal\_functions document under rwsh.excessive\_nesting.
Next it executes the configuration script /etc/rwshrc with
the
arguments that were passed to the shell. So responsibility lies with
/etc/rwshrc for things as basic as appropriately handling -c for when
the shell is used to execute commands for other programs.

The
rwshrc distributed with v0.3 is given below, interspersed with comments.

    .set MAX_NESTING 20
    .function () {.nop}

Again, MAX\_NESTING must be reset, or execution would stop at the first
blank line. The default is currently 20, because in the normal course
of use I have run functions that have nested 17 or 18 levels. The
parentheses are used the way that quotes are in sh and bash.
So
it is defining the function that has no first argument, which is
what is run
when you hit return on a blank line. This and the comment character
must be defined to do nothing, otherwise you get an error about the
executable not being found. (Good luck finding an empty string that is
executable in $PATH.) This might appear to be taking the "do nothing
but provide
a means of configuration" philosophy too far, but you can come up with
cases where redefining each of these would be useful. For example
comments could be replaced with echo commands to provide debugging
printouts. The empty command could be redefined to run its
argument function, or to repeat the previous command (as it does in
gdb). Note that if you do not leave a space after
the pound sign,
it will appear to the shell as a different command (e.g. "#this won't
get ignored"). The fix for this is for rwsh to support wildcards in
function names, so that #* is run whenever any command that begins with
\# is requested, and that particular executable is not in the executable
map. I am not sure when I will get around to implementing
that

    .echo $*0 >/tmp/lcl (
    )

This is useful for debugging shell invocations, especially when it is
some other piece of software that is doing the invocation. $*0 prints
the arguments that the script was invoked with, starting with the name
of the script. $* would skip the name of the script. To include a
newline in the output, it is enclosed in parentheses. It can be
convenient to define a variable (e.g. $nl) to contain a newline, but
this base configuration file is trying to impose as little as possible
on the configuration scripts it will be calling.


    .if .test_equal $# 2 {.source /etc/rwshrc-default}
    .else_if .test_equal $2 -to {.importenv_preserve
      .exec /bin/rwsh test_init.sh <test_main.sh
      .exit}
    .else_if .test_equal $# 3 {.source &2}
    .else_if .test_equal $2 -c {
      rwsh.mapped_argfunction &3 {.source /etc/rwshrc-basic; $1$; .exit}}
    .else {
      .echo unexpected command line: &* (
    )
      .source /etc/rwshrc-default}

If there are no arguments passed by the user ($0 will be the name of
this script, and $1 will be the name under which rwsh was executed),
then /etc/rwshrc-default is used as the configuration. If the first
argument after the name rwsh was executed as is "-to", then the test
script is run, printing all of its output to standard out, and then the
shell exits. If there is some other single argument, then it is used as
the configuration file. (Note if this file contains a .exit command,
then the shell will terminate at the end of the file, before the user
may enter a command.) If there is more than one argument and the first
one is -c, then /etc/rwshrc-basic is used as the configuration, the
second argument is tokenized and then executed, and then the shell
exits. Otherwise, the unrecognized command line is printed with an
error message, and /etc/rwshrc-default is used as a configuration. This
last condition uses the fact that we have configured the blank command
to do nothing, since the space between an open brace and the end of the
line is treated as a command.

### a few words about what happens after you type a command

Commands are divided by newline characters. The first thing that
happens is that it is divided into tokens, separated by one or more
spaces, and anything inside a pair of braces is interpretted as an
argument function. An argument function, like a function in the
executable map, consists of one or more commands divided by semicolons.
If this fails (because the braces are mismatched or there is more than
one argument function for a particular command), then the requested
command is replaced by the appropriate internal function request.

Next, the command is interpretted. Interpretting tokens involves
replacing variables, performing selection reads, and handling escapes.
Variable arguments begin with a $, they are looked up in the variable
map. If a variable does not exist, an error is returned. The variable
map can be queried using the .printenv
built-in. Numbered variables (e.g. $0, $1, $2,...) are positional
parameters. In a function, they correspond to the arguments with with
the function was called, so $0 will be the name of the function, $1
will be the first argument, and so on. In a command line, the
positional parameters refer to the command line. Thus "echo $1 $0 $2"
is interpretted to be "echo $1 echo $0". If the number is preceeded by
a *, then the argument is replaced by that positional parameter and all
of the others following it. As a special case, $* is treated as $*1.
Thus "echo $*3 $* something static" is interpretted as
"echo something static $*3 $* something static". Any argument
that
begins with an @ is interpreted by performing a selection read. This is
explained in the document on the selection. If an argument begins with
a backslash, it is interpretted as an escape, causing a $, space, or \
immediately following it to be handled as a normal character.
An
argument
function is interpretted by reducing argument function levels. See the
argument functions document for an explanation.

The first token on the command line is then looked up in
the executable map. If it is not found, rwsh.autofunction is run. The
token is looked up again in the exeuctable map, and
if if still isn't found, then rwsh.executable\_not\_found is run. If the
first token is found in the executable map, then that executable is
run. If the executable name begins with a forward slash, then the shell
attempts to run
the specified file. In any case the interpretted arguments are passed
to the executable that is run.

### generalized text interface

Part of the idea of this project was to create something of a
generalized text interface. Certain limits were accepted from the
start, such as the idea that a command ends with a newline. This
prevents something based upon rwsh from getting much past the level of
ed as a text editor. Also, any special characters (e.g. braces), which
are interpretted by the shell, cannot be used by commands for other
purposes without being escaped. But as a trivial example, a counter can
be created by
defining the following three commands.

    .set a 0
    .function () {.var\_add a 1}
    .function rwsh.prompt {.echo $a \$}

Empty commands now increment the counter stored in a, and the prompt
prints the current value of the counter. If the following four commands
are used as an init script, then rwsh will treat any command as a
request to increment the given counter.

    .function rwsh.prompt {.printenv}
    .function rwsh.autofunction {
        .set var_name $1
        .if .test_not_empty $$var_name {.var_add $var_name 1}
        .else {.set $var_name 1}
        .set var_name ()}
    .function rwsh.executable_not_found {.nop}

If the first command is replaced by ".function rwsh.shutdown
{.printenv}" then it will count silently, and display the results when
the shell exits. Of course all of these printouts will be cluttered by
the MAX\_NESTING and $? values.
