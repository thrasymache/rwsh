# rwsh.init %source are tested by having test_init.sh define #

# argv tests
%nop
     %nop
%nop 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
%echo 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
%newline
%echo  \    1                2       
%newline
%which_executable rwsh.mapped_argfunction {%nop}
# rwsh.mismatched_brace
%which_executable rwsh.argfunction {
%which_executable rwsh.argfunction }
# rwsh.multiple_argfunctions
%which_executable rwsh.argfunction {} {}
%which_executable rwsh.argfunction {rwsh.argfunction {}}
%which_executable rwsh.argfunction {{{{{{{{{{{}}}}}}}}}}}

# ability of functions to immitate built-ins
%function f {%function $1 {rwsh.argfunction}}
f w {%which_executable $1 {rwsh.argfunction}}
f e {%echo $*; %newline}
w e
w {}
e text that doesn't have a prompt appended

# builtin tests
# %cd
%cd /bin
/bin/pwd

# %echo
%echo these are fixed strings

# %for
%for {e no arguments $1}
%for 1 {e one argument $1}
%for 1 2 3 4 {e four arguments $1}

# %function
%function /bin/bash {%nop}
%function %function {%nop}
%function a {%nop}
%which_executable a
a 1 2 3
%function a
%which_executable a
%function a {e 9 $A $1 @//usr}
%which_executable a
a
a 1
a 1 2
%function a {e $*5 $* $*0}
a
a 1
a 1 2
f g {%function $1 {%function $1 {rwsh.argfunction}}}
g a {e 3 2 1}
w a
a b
b
# a function redefining itself doesn't seg fault
f g {e hi; f g {e there}; f h {e nothing here}; g}
g

# %if %else_if %else
%if %return 0 {e if true; %return 1}
%else {e else true; %return 3}
%if %return 1 {e if false; %return 4}
%else {e else false; %return 5}
%else_if %return 0 {e not this one; %return 6}
%if %return 1 {e nor this; %return 7}
%else_if %return 0 {e but this; %return 8}
%else_if %return 0 {e this should be skipped; %return 9}
%else_if %return 1 {e and certainly this; %return 10}
%else {e nor this; %return 11}
%if %return 0 {%if %return 1 {not to be printed; %return 11}; %else_if %return 0 {e nested else_if; %return 12}}
%else {e else_if failed to appropriately set IF_TEST on exit; %return 13}
%if %return 0 {%else_if %return 0 {e nested syntax; %return 14}}
%else {e already tested; %return 15}

# %if_errno
rwsh.mapped_argfunction {%if_errno {e no error}; %set ERRNO x; %if_errno {e invented error}}

# %internal_errors %internal_features %internal_vars
%internal_errors
%internal_features
%internal_vars

# %ls
%ls /bin /usr/

# %nop
%nop
%nop 1 2 3 4 5

# %printenv %importenv_preserve %set
%printenv
%printenv SHELL
%importenv_preserve
%printenv SHELL
%printenv A
%set A 1
%printenv A

# %return
%return 0
%return 1
%return 1E2

# %selection_set
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

# %stepwise
%set PREV $MAX_NESTING
%set MAX_NESTING 12
f wrapper {a $* one; a $* two; a $* three}
f a {e $* one; e $* two; e $* three}
f d {e $*; %stepwise $* {d $*}}
%stepwise stepwise {e $*}
%stepwise %stepwise {e $*}
wrapper 1 2
%stepwise wrapper 1 2
%stepwise wrapper 1 2 {e $*}
%stepwise wrapper 1 2 {d $*}
%set MAX_NESTING $PREV 
%stepwise wrapper 1 2 {d $*}

# %test_equal %test_not_empty
%test_equal x y
%test_equal x x
%test_not_empty \
%test_not_empty x

# %which_executable %which_test %which_return
%which_test j
%which_executable j
%which_return %which_executable
%which_test #
%which_executable #
%which_return %which_executable
%which_return %which2
%which_executable rwsh.mapped_argfunction {%nop 1 \ \$ \@ $A $0 $# $* $*2 @a @$a @$1 @$* @$*2}
%which_test rwsh.mapped_argfunction
%which_executable rwsh.mapped_argfunction
%which_test rwsh.mapped_argfunction {rwsh.argfunction}
%which_executable rwsh.mapped_argfunction {rwsh.argfunction}

# %while
%function tf {%return $X}
%set X 1
%while tf {e skipped}
%set X 0
%while tf {e printed; %set X 1}
%set X 0
%set Y 0
%while tf {%if %return $Y {%set Y 1}; %else {%set X 1}; e in %while argfunction}

# %version %version_available %version_compatible
%version
%version_available
%version_compatible 1.0

# binary test implicitly tests Old_argv_t
/bin/echo 1 2 3
/bn/echo 1 2 3

# arg_script tests
e 5 4 3 2 1
e $A $0 @$A
e 1 2 $* 3 4
e $*2 1 2
e @//usr
# rwsh.selection_not_found
e @/*is*
e @/bin
e @/usr/*bin
e @/usr/lib*
e @/usr/s*e
e @/usr/*d*
%set FIGNORE include
e @/usr/*d*
e @/usr/*l*i*b*x*e*

# internal functions 
# rwsh.after_command rwsh.raw_command rwsh.prompt
# all of these are used as part of the test itself. If this changes, then the 
# following tests will fail.
w rwsh.after_command
w rwsh.prompt
w rwsh.raw_command

# rwsh.arguments_for_argfunction rwsh.before_command rwsh.binary_not_found
w x {rwsh.escaped_argfunction me}
f rwsh.before_command {%echo $*0; %newline}
/bn
f rwsh.before_command

# rwsh.autofunction %autofunction %which_path
f rwsh.autofunction {%autofunction $1 \$*}
w false
%which_path false
false
w false

# rwsh.executable_not_found
f rwsh.executable_not_found
w rwsh.executable_not_found
w x
x
w rwsh.executable_not_found

# rwsh.mapped_argfunction rwsh.unescaped_argfunction rwsh.argfunction
# rwsh.escaped_argfunction
rwsh.mapped_argfunction 1 2 3 {e a $* a}
rwsh.mapped_argfunction
f g {w rwsh.argfunction {rwsh.unescaped_argfunction}; w rwsh.argfunction {rwsh.argfunction}; w rwsh.argfunction {rwsh.escaped_argfunction}}
g {}

# rwsh.excessive_nesting
f g {h}
f h {g}
g
f rwsh.excessive_nesting {h}
g

# rwsh.run_logic
f rwsh.run_logic {%if %return $1; %else_if $*2; %else}
0 e don't print
1 e do print
1 f rwsh.run_logic
1 e executable not found

# rwsh.vars
rwsh.vars

# %importenv_overwrite
%set SHELL /bin/rwsh
%printenv SHELL
%importenv_overwrite
%printenv SHELL

# exiting rwsh.shutdown
f rwsh.shutdown {e rwsh is now terminating}
%exit
%echo 1 2 3
