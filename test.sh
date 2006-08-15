%set MAX_NESTING 8
%function rwsh.raw_command{%echo $1; %newline}
%function rwsh.prompt{%echo $? \$}
%function #{%true}
%function \ {%true}
# argv tests
%true
     %true
%true 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
%echo 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
%newline
%echo  \    1                2       
%newline
%which rwsh.mapped_argfunction {%true}
%which rwsh.argfunction {
%which rwsh.argfunction }
%which rwsh.argfunction {}{}
%which rwsh.argfunction {rwsh.argfunction{}}
%which rwsh.argfunction {{{{{{{{{{{}}}}}}}}}}}

# ability of functions to immitate built-ins
%function f {%function $1 {rwsh.argfunction}}
f w {%which $1 {rwsh.argfunction}}
f e {%echo $*; %newline}
w e
w {}
e text that doesn't have a prompt appended

# simple builtin tests
%echo these are fixed strings
%false
%false 1 2 3 4 5
%ls /bin /usr/
%printenv
%printenv SHELL
%importenv
%set MAX_NESTING 8
%set IF_TEST
%printenv SHELL
%printenv A
%set A 1
%printenv A
%selection_set A //usr
e $A
%selection_set A /
e $A
%selection_set A local/include
e $A
%selection_set A ..
e $A
%selection_set A \
e $A
%selection_set A /local/../../bin
e $A
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
e 5 4 3 2 1
e $A $0 @$A
e 1 2 $* 3 4
e $*2 1 2
e @//usr
e @/*is*
e @/bin
e @/usr/*bin
e @/usr/lib*
e @/usr/s*e
e @/usr/*d*
%set FIGNORE include
e @/usr/*d*
e @/usr/*l*i*b*x*e*

# function
%function a {%true}
%which a
a 1 2 3
%function a
%which a
%function a {e 9 $A $1 @//usr}
%which a
a
a 1
a 1 2
%function a {e $*5 $* $*0}
a
a 1
a 1 2
f g {%function $1{%function $1{rwsh.argfunction}}}
g a {e 3 2 1}
w a
a b
b
# a function redefining itself doesn't seg fault
f g {e hi; f g {e there}; f h {e nothing here}; g}
g

# control flow
%else_if %true {e not this one}
%set IF_TEST false
%else_if %false {e nor this}
%else_if %true {e but this}
%else_if %true {e this should be skipped}
%else_if %false {e and certainly this}
%set IF_TEST false
%else_if %true {%else_if %true {e nested syntax}; %set IF_TEST false; %else_if %false {not to be printed}; %else_if %true {e nested else_if}; %set IF_TEST false}
%else_if %true {e else_if failed to appropriately set IF_TEST on exit}

# internal functions 
f rwsh.executable_not_found
w rwsh.executable_not_found
w x
x
w rwsh.executable_not_found
rwsh.mapped_argfunction 1 2 3 {e a $* a}
rwsh.mapped_argfunction
f g {w rwsh.argfunction {rwsh.unescaped_argfunction}; w rwsh.argfunction {rwsh.argfunction}; w rwsh.argfunction {rwsh.escaped_argfunction}}
g {}
f rwsh.autofunction{%autofunction $1 \$*}
w false
false
w false
f rwsh.after_command {e after $*0}
f rwsh.after_command
f g {h}
f h {g}
g
f rwsh.excessive_nesting {h}
g

# exiting
%exit
%echo 1 2 3
%function rwsh.after_command {%if_errno {%echo ERRNO set to $ERRNO; %newline; %set ERRNO \ }}
