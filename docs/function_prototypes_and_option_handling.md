# function prototypes and option handling

Function prototypes in rwsh were inspired by the command synopsis
entries found in UNIX man pages. Most of the syntax that I have seen
used in man pages is unambiguous enough for a program to parse, and
they quickly make clear what arguments are acceptable or not to a
command. One specific piece of syntax that cannot be supported is
separating options by a vertical bar (e.g. "[files | directories]"), as
there is no support to restrict what a variable can contain based upon
its contents. Others include using keywords to stand in for unnamed
options, using the same identifier for multiple parameters, or any use
of braces. Three of these can be seen in the following synopsis entry,
along with a violation the current requirement that an elipsis must be
preceeded by a space: "grep [options] [-e PATTERN | -f FILE]
[FILE...]". I take some consolation in that this synopsis entry is
actually a lie, because grep will not accept zero arguments, which is
implied by all of the above parameters being surrounded by brackets.

Currently, functions defined with .function\_all\_flags or are defined
with a function prototype, functions defined by .function are not. When
the prototypes become flexible enough to handle all cases currently in
use, non-prototyped functions will be removed and .function will take
prototypes. Additional builtins to define functions with different command line
expectations may be added later. There will be support for
multiple single-character flag options grouped behind a single dash.
There may be support for other common option-handling paradigms (e.g.
that long-options can occur after some positional arguments, or that
arguments to flag options can be given with an equals-sign). There will
not be support for historical tar style or
similar paradigms, which include rules like all characters following an
"f" in a group of short options are an argument to the -f flag. That
lack of support should only cause function wrappers for such programs
to be more restrictive in the syntax that they will accept than the
programs themselves require, or at least that the functions are
more restrictive in the syntax which they can add functionality to.

### required arguments

Arguments to .function\_all\_flags that follow the function name, are
not enclosed in brackets and are not "..." are interpreted as required
arguments. Given the following definition of a "grep" function,


    .function_all_flags grep regex file {
        .selection_set s $file; &{whichp grep} $regex @$s}

running grep with zero, one, three or more arguments will
produce an error that the wrong number of arguments was passed. If this
grep is run with exactly two arguments, then the named parameters are
filled in as local variables, and can be accessed for the duration of
the function in the same way that global variables are. (Additional
local variables can be defined with the .local builtin.)

### flag options and $-\*

Arguments in the prototype that are surrounded by brackets and begin
with a dash are treated as flag options.  Given the following definition
of a "grep" function,

    .function\_all\_flags grep [-i] regex file {
        .selection_set s $file; &{whichp grep} $-i$ $regex @$s}

running grep will be the same as the two required argument version,
except that additional arguments are accepted, if and only if these
arguments are "-i" and they preceed the two required arguments. The $-i
variable will also be defined for the execution of the function, and it
will contain the text of all the matching flags. It will be the empty
string if the flag is not passed. It will be "-i" if the flag is passed
once, and it will be "-i -i" if the flag is passed twice, etc. The flag
variable is referenced as "$-i$" above because it will avoid passing an
empty string argument to the grep binary if -i was not passed to the
function, and separates the flags if multiple were passed. Multiple
different flags can be handled as well, and are made more convenient by
$-\*. The following two function definitions are roughly equivalent:

    .function_all_flags grep [-i] [-c] [-n] regex file {
        .selection_set s $file; &{whichp grep} $-i$ $-c$ $-n$ $regex @$s}
    .function_all_flags grep [-i] [-c] [-n] regex file {
        .selection_set s $file; &{whichp grep} $-*$ $regex @$s}

The difference between them is that the first will call the grep binary
with the -i options first, the -c options second and the -n options
third, while the second will call the grep binary with the options in
the order in which they were passed. The $-\* local variable is defined
for all functions that take flag options, and is filled in with all of
the flag options that were passed. The fact that the $-\* variable is
not defined for all functions that take zero flag options permits these
functions to make use of the $-\* from the calling function (e.g. in the
argfunction of user-defined control flow).

Flag options can also take arguments:

    .function_all_flags cut [-*] [-b b_list] [-c c_list] [-f f_list] [-d delim] cut file {
        if_only .var_not_empty -d {echo using $delim as delimiter because $-d was passed}
        .selection_set s $file
        &{whichp cut} $-*$ @$s}

This defines a cut which has four flags which take arguments. If any of
those flags are passed, the argument following the flag is assigned as
the value of the local variable named after the flag. In these cases
flag variable will contain not only its own name, but also the argument
that was passed to it. For the cut executable, the -b, -c, and -f
options are mutually exclusive, and the -d option is only meaningful if
-f is passed as well. The corresponding man page on my system expresses
this with three variant commands, each of which has one required flag.
There is currently no support for a "required flag", (i.e. a function
that is defined so that rwsh will throw an exception rather than
running it if it does not have some specific flag passed to it), nor
for variant commands with there being multiple different prototypes
which could be used. So the example above depends upon the binary to
identify these errors.

Parameters that begin with a dash, but are not surrounded by
brackets are treated as ordinary required arguments, and will be filled
in with whatever argument appears at that point in the command line.
This may change in the future. Currently, all flag options must precede all positional arguments. Also
currently, a bare dash is treated as a flag, as this involves the least
special-casing, if the existing uses for dash all end up being as a placeholder
indicating stdin, then this may change.

### [--]

"--" can be used to end the search for flag options so that subsequent
arguments that begin with a dash are treated as positional arguments.
This will happen regardless of whether "[--]" is included in the
function prototype. If an inappropriate number of positional arguments
are encountered, "--" is counted amongst the optional arguments found.
The $-- variable will be set to the value "--" if it occurs in a
command line. If the function has declared flag options, then the "--"
will also be included at the end of the $-\*
variable, whether or not any flag options have been passed on this
command line. If the function has not declared flag options, then only
the $-- variable will be set. Note that if "--" occurs more than once
in the command
line that the first occurrence will be recognized as terminating the
flag arguments and any subsequent occurrences will be treated as
positional arguments. Also note that because "[--]" is implicitly in
every function prototype, "--" cannot be a required parameter.

### [-\*]

Functions that do not have [-\*] in their prototype will throw an
rwsh.unrecognized\_flag flag exception if they are called with a flag that has
not been declared, while functions that do have [-\*] in their prototype will
not. All functions will only define local variables for those parameters that
have been declared, but undeclared flags are kept in the variable $-? and all
flags are kept in the variable $-\*. So non-declared options are accessible in
one of those two places (presumably to be passed on to some executable that
takes more options than the function needs to have special handling for).
Because there is an expectation that non-declared flag options will be passed to
and will be significant to functions declared with [-\*], $-\* is defined for
all such functions, regardless of whether or not they explicitly declare any
flag options, but this is not true of functions that declare no flag options
themselves (which permit said functions to operate on the flag options of the
function that calls them). Note that [-\*] cannot trigger appropriate handling
of flags that have not been declared that also take options. 

### optional positional arguments (non-flag options)

Arguments in the prototype that are surrounded by brackets that do not
begin with a dash are treated as optional positional arguments, and
arguments given to the function in the corresponding positions will
be the values given to these variables. Note that local variables corresponding
to optional positional arguments will not be defined if the optional argument is
not present.  The empty string can be passed as an argument, either literally as
"()", or as the value of a variable which itself contains the empty
string. If and only if the empty string is passed as an argument that
is used for a non-flag option, then the variable will be defined but
empty.

It is most common for optional positional arguments to be
individual arguments and follow the required arguments, but neither of
these is necessary. Optional positional arguments will be filled in in
the order in which they are encountered, while reserving enough
arguments to fill in the required arguments. Given the following pair of functions:

    .function_all_flags iee var {
        .if .var_exists $var {.echo $$var}
        .else {.nop}}
    .function_all_flags foo [a] [b] c [d] {
        echo ${iee a} ${iee b} ${iee c} ${iee d}}

The following will occur:

    $foo
    unacceptable arguments: for prototype [a] [b] c [d],
     needed (c),
     call stack: foo rwsh.mapped_argfunction
    foo returned -1

    $foo one
    () () one ()

    $foo one two
    one () two ()

    $foo one two three
    one two three ()

    $foo one two three four
    one two three four

    $foo one two three four five
    unacceptable arguments: for prototype [a] [b] c [d],
     assigned a=one b=two c=three d=four ,
     but found (five) excess
     call stack: foo rwsh.mapped_argfunction
    foo returned -1

Multiple optional positional arguments that are grouped together within a
single set of brackets are filled or not filled as a group. If a point is
reached where there are too many arguments for the preceeding optional groups
and the required parameters, but not enough for the current optional group, then
the rwsh.bad\_args error is thrown, this is true even if there is a subsequent
optional group which could be filled. For example, if the above function `foo`
was replaced with the following:

    .function_all_flags foo [a b c] [d e] f {
        echo ${iee a} ${iee b} ${iee c} ${iee d} ${iee e} ${iee f}}

Then the following will occur:

    $foo
    unacceptable arguments: for prototype [a b c] [d e] f,
     needed (f),
     call stack: foo rwsh.mapped_argfunction
    foo returned -1

    $foo one
    () () () () () one

    $foo one two
    unacceptable arguments: for prototype [a b c] [d e] f,
     assigned a=one b=two ,
     but needed (c f),
     call stack: foo rwsh.mapped_argfunction
    foo returned -1

    $foo one two three
    unacceptable arguments: for prototype [a b c] [d e] f,
     assigned a=one b=two c=three ,
     but needed (f),
     call stack: foo rwsh.mapped_argfunction
    foo returned -1

    $foo one two three four
    one two three () () four

    $foo one two three four five
    unacceptable arguments: for prototype [a b c] [d e] f,
     assigned a=one b=two c=three d=four e=five ,
     but needed (f),
     call stack: foo rwsh.mapped_argfunction
    foo returned -1

    $foo one two three four five six
    one two three four five six

    $foo one two three four five six seven
    unacceptable arguments: for prototype [a b c] [d e] f,
     assigned a=one b=two c=three d=four e=five f=six ,
     but found (seven) excess
     call stack: foo rwsh.mapped_argfunction
    foo returned -1

Note that because optional arguments that begin with a dash are
interpreted as flag options, it is not possible to name an optional
positional argument beginning with a dash.

### ...

"..." occurring in the definition of a function is used to
indicate additional arguments are possible. It does not fill in a
corresponding $... local variable, but rather appends them to the
previous argument, as this is the usage that I find in man pages. This
argument may or may not be enclosed in brackets, depending upon whether
it is required or optional. To continue with the variants on grep
example:

    .function_all_flags grep regex file ... {
        .selection_set s $file$; &{whichp grep} $regex @$s}
    .function_all_flags grep regex [file ...] {
        .if .var_exists file {.selection_set s $file$}; &{whichp grep} $regex @$s}

The first version takes two or more arguments, with at least one path
name being required, but more than one being accepted, all are stored
in the local variable $file. The second version makes specifying files
optional, and so it takes one or more arguments. If these functions are
passed two or more arguments then they behave identically. If they are
passed one argument, then the first version fails to run with a bad
argument count exception being thrown, but the second version runs with
an unmodified selection. "..." and "[...]" have the same meaning, which
is to say that the first version above does not require two or more
arguments. "..." is currently not permitted to occur more than once in
a single function prototype. Note that a space is currently required
before
the "...", though I have seen man pages that do not do this.

### Whitespace within brackets
To simplify the parsing logic in the initial implementation, whitespace
can only be used within brackets to separate parameters, which is
consistent with a majority of the man pages that I have seen. As a
result, whitespace following an open bracket or preceding a close
bracket is interpreted as declaring the empty string parameter (i.e.
the variable that is accessed with a bare $), and thus having
whitespace after the opening bracket and following the closing bracket
is defining two separate parameters both named the empty string which
is an error (though either by themselves is acceptable). I find the
declarations to be clearer without extra whitespace around the
brackets, but since the implication in the man pages that I have seen
is that there only be as many parameters as there are identifiers, and
since the rarity of support for variables named the empty string makes
this behavior surprising, and since there is no need to make it extra
convenient to declare parameters named the empty string, this behavior
is likely to change in the future. It may change to generate a clearer
error in these situations rather than to ignore this extra whitespace.
As elsewhere, tokens may be separated by multiple spaces or a single
space.

### Unused and unchecked variables
It is an error for the body of the function to not match its prototype.
For this reason, an exception is thrown for each defined variable whose value is
not used whether it is a parameter or a local variable.  This also happens for
global variables that have never been used in the life of the shell.  In
addition, an exception is thrown for each optional argument declared in the
prototype whose existence is not checked with .var\_exists, `$-*`, or
.list\_locals.

 Having `$var_name` be an argument to any statement that is executed uses/reads
the variable `var_name`; this includes `.nop $var_name` and `# $var_name`, but
`.if test {g $var_one}; .else {g $var_two}` will only use/read `var_one`
if `test` returns true, and only use/read `var_two` if `test` returns false.

Flag variables that do not take arguments (e.g.  `[-x]`) are a special case in
that if they are defined, their value is one or more repetitions of their name,
so checking whether they exist is considered to use that value as well (even if
the check is simply `.list_locals` being called).

The convenience variables `?` and `--` do not need to be used or checked. In
addition, using some other convenience variables implicitly uses other variables
(and visa versa). There are also ways to implicitly check variables, as
described below. (It would be rather pedantic to require otherwise.)

#### Ways of using or checking multiple variables at once
The `-*` variable is special. It contains all of the flag options with any
arguments. The requirement that parameters be used or checked means that either
those flags may be used or checked individually, or they may be used and checked
collectively by making use of `$-*`. It is not necessary to do both for flag
arguments, but `$-*` does not contain any positinal arguments, so they must be
checked and used even if `$-*` is used.

The `.list_locals` built-in produces as output the names of all of the defined
parameters and local variables. So there is no need to check the existence of
any variables after `.list_locals` is run, but it is still necessary to make use
of the variables returned that are not simple flags.

A group of positional arguments (e.g. `[one two three four]`) will either all
have been passed or none will have been passed (otherwise the function won't run
because a bad arguments error was returned to the caller). For this reason it is
not necessary to check each of the members of the group - checking one is
sufficient to check all of them. However, if they are defined, then each of them
must be used, or an unused variable exception will be thrown for that
parameter.

The rules for flag variables with one or more arguments is an extension of the
rules for groups of positional arguments. If we define the following function:

    .function_all_flags f [-x foo bar] {

and call it as

    f -x one two

then `$-x`  has the value `-x one two`, `$foo` has the value `one` and `$bar`
has the value `two`. If we call f without arguments, then none of the `-x`,
`foo`, or `bar` variables are defined. So, checking any of the three is
sufficient to check all three. If `$-x` is used, then neither `foo` nor `bar`
needs to be used, since their values are contained inside `$-x`. Similarly, if
`$foo` and `$bar` are both used, then there is no need to use `$-x`, because the
only parts of it not contained in `$foo` and `$bar` are whitespace and the `-x`
without which `foo` and `bar` would not be defined.

For example, consider the following function definition

    .function_all_flags f [-x arg] [-y] [foo bar baz] {}

With an empty body it would always throw unused variable exceptions for all of
the arguments that were passed and unchecked variable exceptions for all of the
arguments that are not passed.  However, it would be sufficient for the body to
contain `if_only .var_exists arg -y baz {}` to avoid the unchecked variable
exceptions. This would not prevent unused variable exceptions if arguments were
passed to the function. However,

    {if_only .var_exists arg {echo $arg}
         if_only .var_exists -y {echo y flag passed}
         if_only .var_exists bar {echo $foo $bar $baz}} 

is one minimal body that would avoid unchecked and unused variable exceptions.

    {echo $-*
         if_only .var_exists foo {echo $foo $bar $baz}}

is another body that would avoid unchecked and unused variable exceptions. Note
that `$-*` only contains the flag options, and so the positional arguments
`foo`, `bar`, and `baz` must be checked and used separately. Finally,

    {.list_locals}

as the complete body would avoid any unchecked variable errors, but would
produce an unused variable error for each parameter that was defined other than
`-y`. Instead,

    {.for ${.list_locals} {echo $1}}

would avoid both unused and unchecked variable errors.

Obviously, these exceptions were included in `rwsh` because the vast majority of
expected functions will check and use their parameters without any additional
code, and will generally make much better use of their variables than the
examples above.  As with other uncommon functionality, the `rwsh` init scripts
are expected to provide a wide set of examples of how to make effective use of
`rwsh`.

#### The meaninglessness of existence checking for `$-?` and `$-*`
If `[-?]` occurrs in a function prototype, then `$-?` or `$-*` or both must be
run in the body.  If the example function from earlier had instead been 

    .function_all_flags f [-?] [-x arg] [-y] [foo bar baz] {
        if_only .var_exists -? -* arg -y baz {}}

then unused variable exceptions would be thrown for `-?` and `-*` regardless of
the arguments. While this may be counter-intuitive, the fact is that both `-?`
and `-*` will exist regardless of the arguments, they will merely have the empty
string as their value if no flag arguments are passed. So checking their
existence provides no information about the arguments. The earlier body with
`echo $-*` and the one that looped over the output of `.list_locals` would avoid
these errors, as would the following body.

    if_only .var_exists arg {echo $arg}
         if_only .var_exists -y {echo y flag passed}
         if_only .test_not_empty $-? {echo other flags passed}
         if_only .var_exists bar {echo $foo $bar $baz} 

