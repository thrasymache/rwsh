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

<pre>.function fn {.function $* {rwsh.argfunction}}</pre>


the result of interpretting the command line results in<br>

<pre>.function fn {.function $* {rwsh.unescaped\_argfunction}}</pre>

which places ".function $* {rwsh.unescaped\_argfunction}" in the
executable\_map with name fn.<br>

<br>

if you then type <br>

<pre>fn sel {echo $SELECT}</pre>


the command is unchanged by interpretation, and the function that is
stored under fn gets interpretted to be<br>

<pre>.function sel {echo $SELECT}</pre>


If we had instead typed<br>

<pre>.function fn {.function $* {rwsh.unescaped\_argfunction}}</pre>


It would have been interpretted to result in<br>

<pre>.function fn {.function $* {.function $* {rwsh.unescaped\_argfunction}}}</pre>


The result would have been a function that takes an argument function
and defines a second function which, when run, would define a function
to execute the argument function that was passed to the first function.<br>

<br>

The which command seeks to describe what is in the executable map in
the terms that you would use to put it there rather than what is
actually there. Thus<br>

<pre>which fn</pre>


prints out<br>

<pre>.function fn {.function $* {rwsh.argfunction}}</pre>


instead of what is stored, which is<br>

<pre>.function $* {rwsh.unescaped\_argfunction}</pre>


The which command also recognizes the names rwsh.mapped\_argfunction and
rwsh.argfunction to request the printing of the specified argument
function. In these cases the ".function rwsh.argfunction " part of the
print out is omitted for various reasons. For example, we could define
fn\_v<br>

<pre>fn fn\_v {.echo adding \ ; which rwsh.argfunction {rwsh.argfunction};<br>.echo \ to executable map as $1; fn $* {rwsh.argfunction}}</pre>


then running<br>

<pre>fn\_v sel {echo $SELECT}</pre>


would print out<br>

<pre>adding {echo $SELECT} to executable map as sel</pre>


<h3>rwsh.escaped\_argfunction</h3>

Suppose you wanted to define a function fn-fn such that it defined a
function with the name of its first argument which was equivalent to
.function. It needs to incorporate the
argument function, but not its own argument function, the new
function's argument
function. The solution is rwsh.escaped\_argfunction, which is replaced
by rwsh.argfunction when interpretted. fn-fn can be defined as follows:<br>

<pre>.function fn-fn {.function $1 {.function $* {rwsh.escaped\_argfunction}}}</pre>


<h3>argfunction call as the result of an interpretation</h3>

If you gave the command<br>

<pre>rwsh.mapped\_argfunction {echo x; rwsh.mapped\_argfunction<br>rwsh.argfunction {$1}}</pre>


There is a problem: if rwsh.argfunction was given directly as an
executable, since rwsh.argfunction is only recognized as an executable
name when the shell is trying to execute $1, it does not get
interpretted when there is any argument function for it to be replaced
with. &nbsp;As a result, only rwsh.mapped\_argfunction is supported
as
the result of an interpretation<br>

<pre>rwsh.mapped\_argfunction rwsh.mapped\_argfunction xyy {$1 $2 {echo $1}}</pre>


will print<br>

<pre>xxy</pre>

</body>
</html>
