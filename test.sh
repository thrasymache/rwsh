%true
     %true
# sanity check and argv tests
%true 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
%echo 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
%echo  \    1                2       
%newline
%which rwsh.mapped_argfunction {%true}
%which rwsh.argfunction {
%which rwsh.argfunction }
%which rwsh.argfunction {}{}
%which rwsh.argfunction {rwsh.argfunction{}}
%which rwsh.mapped_argfunction {{{{{{{{{{{}}}}}}}}}}}
%function rwsh.before_command{echo $*0}

# simple builtin tests
%echo these are fixed strings
%false
%false 1 2 3 4 5
%ls /bin /usr/
%printenv
%printenv TERM
%importenv
%printenv TERM
%printenv A
%set A 1
%printenv A
%selection_set A //usr
echo $A
%selection_set A /
echo $A
%selection_set A local/include
echo $A
%selection_set A ..
echo $A
%selection_set A \
echo $A
%selection_set A /local/../../bin
echo $A
%true
%true 1 2 3 4 5
%which a
%which #
%which rwsh.mapped_argfunction {%true 1 \ \$ \@ $A $0 $# $* $*2 @a @$a @$1 @$* @$*2}
%which rwsh.mapped_argfunction {rwsh.argfunction}

# binary test implicitly tests Old_argv_t
/bin/echo 1 2 3
/bn/echo 1 2 3

# arg_script tests
echo 5 4 3 2 1
echo $A $0 @$A
echo 1 2 $* 3 4
echo $*2 1 2
echo @//usr
echo @/*is*
echo @/bin
echo @/usr/*bin
echo @/usr/lib*
echo @/usr/s*e
echo @/usr/*d*
%set FIGNORE include
echo @/usr/*d*
echo @/usr/*l*i*b*x*e*

# function
%function a {%true}
%which a
a 1 2 3
%function a
%which a
%function a {echo 9 $A $1 @//usr}
%which a
a
a 1
a 1 2
%function a {echo $*5 $* $*0}
a
a 1
a 1 2
# first test of fn function rather than function builtin
fn e {%function $1{%function $1{rwsh.argfunction}}}
e a {echo 3 2 1}
# first test of function which rather than builtin which
which a
a b
b
# a function redefining itself doesn't seg fault
fn f {echo hi; fn f {echo there}; fn g {echo nothing here}; f}
f

# control flow
%elif %true {echo not this one}
%set IF_TEST false
%elif %false {echo nor this}
%elif %true {echo but this}
%elif %true {echo this should be skipped}
%elif %false {echo and certainly this}
%set IF_TEST false
%elif %true {%elif %true {echo nested syntax}; %set IF_TEST false; %elif %false {not to be printed}; %elif %true {echo nested elif}; %set IF_TEST false}
%elif %true {echo testing if elif appropriately sets IF_TEST on exit}

# internal functions 
fn rwsh.executable_not_found
which rwsh.executable_not_found
which x
x
which rwsh.executable_not_found
rwsh.mapped_argfunction 1 2 3 {echo a $* a}
rwsh.mapped_argfunction
fn f {which rwsh.argfunction {rwsh.unescaped_argfunction}; which rwsh.argfunction {rwsh.argfunction}; which rwsh.argfunction {rwsh.escaped_argfunction}}
f {}
fn rwsh.autofunction{%autofunction $0 \$*}
which false
false
which false
fn rwsh.after_command {echo after $*0}
fn rwsh.after_command
%set MAX_NESTING 5
fn f {g}
fn g {f}
f
fn rwsh.excessive_nesting {g}
f

# exiting
exit
echo 1 2 3
