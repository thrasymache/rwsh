<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en-us">
<head>
  <meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">
  <title>the selection</title>
</head>
<body>

Most of my time using a shell involves searching for some files and
then doing something with them. As a somewhat contrived example:

    ls /usr/local
    ls /usr/local/teTeX
    ls /usr/local/teTeX/include
    ls /usr/local/teTeX/include/kpathsea
    ls /usr/local/teTeX/include/kpathsea/c-*
    less /usr/local/teTeX/include/kpathsea/c-*

One might repetitively use cd in order to keep the paths shorter, or
use readline features to edit the previous command, but both distract
from what you're trying to do. For contrast, the corresponding, normal
rwsh sequence would be

    ls /usr/local/
    ls teTeX/
    ls include/
    ls kpathsea/
    ls c-*
    less

The argument to each command adds to the previous selection.
(The /
before "usr" clears the previous selection.) Finally, since the desired
files are
already selected, less does not need any arguments. This is
accomplished by each command being a shell function using selection set
and selection read. Both ls and less could be defined as follows:

    function ls {.selection_set s $1; /bin/ls -d @$s}
    function less {.selection_set s $1; /usr/bin/less
    @$s}

.selection\_set takes a shell variable as its first argument, it uses
the pattern in its second argument to augment the pattern held in the
shell variable. If no argument is given, then the existing pattern is
not changed. @ is the selection read operator, which converts the
pattern into a list of the files matched by that pattern. @ can be
followed by a text string or $ and a shell variable. There is nothing
special about the variable s, except that both of the functions
use it to store the regular expression that they get their file list
from.

Since this accomplished using a normal shell variable, it is trivial to
implement commands like ftp's lls to explore a second selection, while
leaving the first unchanged. Since the filename matching patterns are
only treated specially by .selection\_set and selection read, grep could
be defined by

function grep {.set REGEX $1; .selection\_set s $2 $*3; /usr/bin/grep
$REGEX @$s} The first argument is neither used by .selection\_set nor
selection read
and so it can contain the wildcards that are used in filename patterns,
and pass unchanged to grep as long as it doesn't start with a
dollar sign or open brace. It is stored in the variable REGEX so that
another function, regrep, can be defined to grep for the same regular
expression in a different list of files. The second
argument, on the other hand makes full use of filename patterns. The
issue that this does not permit passing options to grep will be fixed
in a future release. The current workaround is to define a third shell
function, grepo, that takes options as its first argument, a regular
expression as its second argument, and optionally a list of files as
subsequent arguments.

The user interface of grep can be improved beyond removing the need to escape
the regular expression used for pattern matching. grep can be used to
modify the selection that will be used by subsequent commands. The
version that I have been using involves the following two functions:

    .function grepl {
        .set REGEX $1
        .selection_set s $2 $*3
        .set grep-sel $s
        .selection_set grep-sel &&{&{whichp grep} -l $REGEX @$s}$
        echo $grep-sel}
    .function gs {.set s $grep-sel}

This sets the variable grep-sel to be the list of files that match the
pattern, if the list appears correct, then this can be made the active
selection by running gs. This is only one of many possible
implementations: if efficiency is not an issue (you are always looking
at a small amount of data), then the statement that modifies grep-sel
can be added to normal grep commands, running grep twice. A more
complicated implementation could parse the output of grep to identify
the files mentioned, and set the variable accordingly.

@ (selection read)

 * Steps in a filename pattern are text separated by forward
slashes ("/").
 * If a step consists of "..", the effect is to undo the
previous
step. ("." is not handled specially, but works normally because it is
part of the file system. There is currently no way within a selection
read to refer to the file
link in each directory named "..".)
 * Otherwise a step may consist of any combination of text and
star
("*") characters. Text matches itself, a star matches zero or more of
any character in a filename. ("?" and other metacharacters are not
supported at this time.)
 * If there is an empty step (other than
after the final slash), that is if there are two consecutive
forward slashes at any point in a filename pattern, the effect is to
move to the root of the file system. Thus @/usr/local//bin is
equivalent to @/bin.
 * If the end of a patten is an empty step (i.e. the pattern
ends
with a forward slash), the pattern is read as if the final step was a
star. Thus "@/usr/" is equivalent to "@/usr/*"

.selection_set &lt;variable&gt; &lt;pattern&gt;

&lt;variable&gt;
is converted into a filename pattern and then &lt;pattern&gt;
is added
to it. The last part of the pattern in &lt;variable&gt; is
replaced by
the first part of &lt;pattern&gt;, unless
&lt;pattern&gt; begins with a
/. As with selection read, a pattern that ends with a / is read as if
it ended with /*. 

 * If &lt;variable&gt; contains "/usr/local", and
&lt;pattern&gt; is "bin", the result is "/usr/bin".
 * If &lt;variable&gt;
instead contains "/usr/local/", it is read as "/usr/local/*", and the
result is "/usr/local/bin".
 * If &lt;variable&gt; again contains
"/usr/local", but &lt;pattern&gt; is "./bin", then the result
is
"/usr/local/bin".
 * If &lt;variable&gt;
contains "/usr/local/bin" and
&lt;pattern&gt; is "..", then the ".." replaces the "bin", and
the
effect of the ".." is to remove "local", resulting in "/usr".
 * Similarly if &lt;variable&gt; contains
"/usr/local/bin" and &lt;pattern&gt; is "../bin", the result is
"/usr/bin".

</body>
</html>
