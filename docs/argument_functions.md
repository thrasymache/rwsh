# argument functions

Along with normal text arguments, commands can include argument
functions enclosed in braces. For example,

    .function sel {echo $SELECT}

The function builtin is passed "sel" as $1 and "echo $SELECT" as an
argument function. The function builtin creates a new function in the
executable map using $1 for the name and the argument function for the
content. Braces are recognized as special characters. Currently, the
close brace does not need
any whitespace around it, though the open brace must be preceeded by
whitespace to distinguish it from a substitution. It is likely that the
close brace will require whitespace following it to make it
easier/possible to include the character in normal arguments. The
argument function may also occur anywhere in the command
line, not just at the end (e.g. ".function {echo $SELECT}sel" is
equivalent to the command given above.) Empty braces ("{}") refer to
the same empty command that a blank line does.

There are also four constructs named like internal functions to permit
user-defined functions to use argument functions themselves:
rwsh.mapped\_argfunction, rwsh.unescaped\_argfunction, rwsh.argfunction
and rwsh.escaped\_argfunction. Each of these is used as an executable
name to get the special meaning. Since they are not keywords, they can
be used as arguments to commands without the shell interfering. Of them
only rwsh.mapped\_argfunction can take arguments itself.

### rwsh.mapped\_argfunction

rwsh.mapped\_argfunction executes its argument function with the given
arguments. For example

    rwsh.mapped_argfunction z c {echo x y $1; echo a b $2; echo $0}

will output

    x y z
    a b c
    rwsh.mapped_argfunction

Nested argument functions are possible, and variables are not
interpretted for argument functions until they are run. For example

    rwsh.mapped_argfunction 1 2 3 {
        echo $*
        rwsh.mapped_argfunction a b $1 {
            echo $*
            rwsh.mapped_argfunction x $2 $3 {echo $*}}}

will output

    1 2 3
    a b 1
    x b 1

### rwsh.unescaped_argfunction and rwsh.argfunction

Suppose that you wanted a function to provide a shorter name for
.function. What you need is a way of passing the argument function for
the current function an an argument function to the command that it
calls. Each time an argument script is interpretted
rwsh.unescaped\_argfunction is replaced by the current argument function
and rwsh.argfunction is replaced by rwsh.unescaped\_argfunction.

so if you type

    .function fn {.function $* {rwsh.argfunction}}

the result of interpretting the command line results in

    .function fn {.function $* {rwsh.unescaped_argfunction}}

which places ".function $* {rwsh.unescaped\_argfunction}" in the
executable\_map with name fn.

if you then type

    fn sel {echo $SELECT}

the command is unchanged by interpretation, and the function that is
stored under fn gets interpretted to be

    .function sel {echo $SELECT}

If we had instead typed

    .function fn {.function $* {rwsh.unescaped_argfunction}}

It would have been interpretted to result in

    .function fn {.function $* {.function $* {rwsh.unescaped_argfunction}}}

The result would have been a function that takes an argument function
and defines a second function which, when run, would define a function
to execute the argument function that was passed to the first function.

The which command seeks to describe what is in the executable map in
the terms that you would use to put it there rather than what is
actually there. Thus

    which fn

prints out

    .function fn {.function $* {rwsh.argfunction}}

instead of what is stored, which is

    .function $* {rwsh.unescaped_argfunction}

The which command also recognizes the names rwsh.mapped\_argfunction and
rwsh.argfunction to request the printing of the specified argument
function. In these cases the ".function rwsh.argfunction " part of the
print out is omitted for various reasons. For example, we could define
fn\_v

    fn fn_v {.echo adding \ ; which rwsh.argfunction {rwsh.argfunction};
    .echo \ to executable map as $1; fn $* {rwsh.argfunction}}

then running

    fn_v sel {echo $SELECT}

would print out

    adding {echo $SELECT} to executable map as sel

### rwsh.escaped_argfunction

Suppose you wanted to define a function fn-fn such that it defined a
function with the name of its first argument which was equivalent to
.function. It needs to incorporate the
argument function, but not its own argument function, the new
function's argument
function. The solution is rwsh.escaped\_argfunction, which is replaced
by rwsh.argfunction when interpretted. fn-fn can be defined as follows:

    .function fn-fn {.function $1 {.function $* {rwsh.escaped_argfunction}}}

### argfunction call as the result of an interpretation

If you gave the command

    rwsh.mapped_argfunction {echo x; rwsh.mapped_argfunction
    rwsh.argfunction {$1}}

There is a problem: if rwsh.argfunction was given directly as an
executable, since rwsh.argfunction is only recognized as an executable
name when the shell is trying to execute $1, it does not get
interpretted when there is any argument function for it to be replaced
with. As a result, only rwsh.mapped\_argfunction is supported
as
the result of an interpretation

    rwsh.mapped_argfunction rwsh.mapped_argfunction xyy {$1 $2 {echo $1}}

will print

    xxy
