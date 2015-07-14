.return 0
# .init is tested by having test_init.sh define #

# argv tests
.nop
     .nop
.nop 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
.echo 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
.echo  ()    1                2       $#
.echo \escaped internal\ space $#
.echo now \ external\  $#
.echo a \  space $#
.echo \$tokens \} \{ \; \\ \) \(
.echo a \
line continuation
 .echo ignore leading space
	.echo ignore leading tab
.which_executable rwsh.mapped_argfunction {.nop}
.which_executable rwsh.argfunction {
  multiple line argfunction }
.nop rwsh.argfunction rwsh.mismatched_brace } 
.nop multiple statements \; on a line
.which_executable rwsh.argfunction {rwsh.multiple_argfunctions} {}
.which_executable rwsh.argfunction {rwsh.argfunction with text args}
.which_executable rwsh.argfunction {rwsh.argfunction {with a function arg}}
.which_executable rwsh.argfunction {{{{{{{{{{{}}}}}}}}}}}

# ability of functions to immitate built-ins
.function f {.function $1 {rwsh.argfunction}}
f w {.which_executable $1 {rwsh.argfunction}}
f e {.echo $*}
f m {rwsh.argfunction}
.which_executable f
.which_executable w
.which_executable e
.which_executable m
w e
w () {}
e text that doesn't have a prompt appended
m {e again}
.function_all_options if_only first_argument second_argument {
  .if $first_argument $second_argument {rwsh.argfunction}; .else {.nop}}
if_only .return 1 {e not printed}
if_only .return 0 {e printed without error}
.function for {
  .if .var_exists $1 {.for &&*1$ {rwsh.argfunction}}; .else {.nop}}
for {e skipped without error}
for 1 2 3 {e loop $* $nl}

# arg_script tests
.set A /bin
e 5 4 3 2 1
e $A $0 @$A
e 1 2 $* 3 4
e $*2 1 2
e A $1 1 $$3 $$$3
e A 1 2 3 4 5 6 7 $$$$$$$$$8
e &&A
m {e &&&A}
e &{e &&A}
e &&{e &A}
e &A
.if .nop 1 2 3 {e &*}
.else {}
.if .nop 1 2 3 {e &*0}
.else {}
m {.set A not_bin; e &A &&A $A $nl; m {.set A otherwise; e &A &&A &&&A $A}}
.set A /bin
m {.set A not_bin
   e &{.echo $A} &&{.echo $A} $A $nl
   m {.set A otherwise
      e &{.echo $A} &&{.echo $A} &&&{.echo $A} $A}}
.set A /bin
m {.set A not_bin; e &{.echo &A} &&{.echo &A &&A}}
m &{.echo $A} {e $1 &1}
m $FOO {}
m {e $FOO}
m &{.return 1} {}
m {e &{.return 1}}
m {e &&{.return 1}; e after}
f rwsh.failed_substitution {e &&{.return 1}}
m {e &&{.return 1}; e after}
f rwsh.failed_substitution
# bad_argfunction_style
e x{e x}
e @/etc
# rwsh.selection_not_found
e @/*is*
e @test_main.cc
e @e*c
e @test_files/*xx
e @test_files/*x*x*x*x
e @test_files/*xyxy
e @/bin
e @/usr/*bin
e @/etc/rwsh*
e @/etc/rw*ic
e @/etc/rwsh*a*
.set FIGNORE *de*
e @/etc/rwsh*a*
e @/usr/*l*i*b*x*e*
e @test_main.sh
e @*hrc*
.set A r*h.cc sel*.h
e @$A
.return &{.return 0}
.return &{e 0 $nl}
.return &{.echo 0}
e nevermore &{/bin/echo quoth the raven} 
.set A ((zero zero) (one one) two three)
m $A$$ {e $* $#}
m $A {e $# $*}
m $A$ {e $# $*}
m $A$1 {e $# $*}
m $A$10 {e $# $*}
m &{e ((zero zero) (one one) two three)}@ {e $# $*}
m &{e ((zero zero) (one one) two three)}$$ {e $# $*}
m &{e ((zero zero) (one one) two three)} {e $# $*}
m &{e ((zero zero) (one one) two three)}$ {e $# $*}
m &{e ((zero zero) (one one) two three)}$1 {e $# $*}
m &{e ((zero zero) (one one) two three)}$10 {e $# $*}
m &{e (zero zero) \)one one two three}$1 {e $# $*}
m &{e (zero zero) \)one one two three} {e $# $*}
e a (tight string created by parentheses $#) $#
e a ( spaced string created by parentheses $# ) $#
e some escaped \) \(parentheses $#
e some (nested (parentheses) $#) $#
e some ((((((((((repeated))))) parentheses))))) $#
e a (multi-line parenthesis
  enclosed string) $#
e a )mismatched parenthesis
e a (multi-line parenthesis
  mismatch))
e (internal \)parenthesis \\ escape ) $#
.unset A

# file redirection (but don't overwrite files that exist)
# .for_each_line
.if .ls dummy_file {.exit}
.else {}
/bin/cat <dummy_file
.for_each_line <dummy_file {e line of $# \( $* \)}
m {e hi >dummy_file >another}
m {e hi >dummy_file}
/bin/cat dummy_file
.if .return 0 {>dummy_file /bin/echo there}
.else {.nop}
/bin/cat dummy_file
m {m >dummy_file {e line 1 $nl; e line 2 longer $nl; .echo $nl; e ending}}
/bin/cat <dummy_file
.for_each_line x {}
.for_each_line <dummy_file
.for_each_line <dummy_file <another {}
.for_each_line <dummy_file {e line of $# \( $* \) $nl}
/bin/rm dummy_file

# soon level promotion
.global A 0
.global OLD_NESTING $MAX_NESTING
.set MAX_NESTING 46
f x {.var_add A 1
     m {.var_add A 1
        m {.var_add A 1
           m {.var_add A 1
              m {.var_add A 1
                 m {rwsh.argfunction}}}}}}
x {e &A &&A $A}
.set A 0
x {x {x {x {e &A &&A &&&A &&&&A &&&&&A $A}}}}
.set A 0
x {e &{.echo &A $A} &&{.echo &A &&A $A} $A}
.set A 0
x {x {x {x {e &{.echo &A $A} &&{.echo &A &&A $A} &&&{.echo &A &&A &&&A $A} &&&&{.echo &A &&A &&&A &&&&A $A} &&&&&{.echo &A &&A &&&A &&&&A &&&&&A $A} $A}}}}
f x
.set MAX_NESTING $OLD_NESTING
.unset A
.unset OLD_NESTING

# builtin tests
# .cd
.cd
.cd /bin {excess argfunc}
.cd /bin /
.cd /bin
/bin/pwd

# .combine
.combine
.combine something {excess argfunc}
.combine on () e \ two ( ) $#

# .echo
.echo
.echo something {excess argfunc}
.echo these are fixed strings

# .exec .fork
.fork
.fork e text
.fork .return 1
.exec
.exec something {excess argfunc}
.fork m {.exec /bin/echo something; /bin/echo else}

# .for
.for {e no arguments $1}
.for no_argfunction
.for 1 {e one argument $1}
.for 1 2 3 4 {e four arguments $1 $nl}

# .function
.function
.function /bin/echo {.echo $* $nl}
.function .exit {.nop}
.function rwsh.escaped_argfunction {.nop}
.function a
.function a {.nop}
.which_executable a
a 1 2 3
.function a
.which_executable a
.global A \
.echo () A () A () $#
.function a {e 9 $A $1 @/usr}
.which_executable a
a \
a 1
a 1 2
.function a {e $*5 $* $*0 $nl}
a
a 1
a 1 2
f g {.function $1 {.function $1 {rwsh.argfunction}}}
g a {e 3 2 1 $nl}
w a
a b
b
# a function redefining itself doesn't seg fault
f g {e hi $nl; f g {e there $nl}; f h {e nothing here}; g}
g

# .function_all_options
.function_all_options
.function_all_options rwsh.argfunction {e cannot define rwsh.argfunction}
.function_all_options a {e zero argument function acceptable}
a
.function_all_options test_var_greater
.function_all_options test_var_greater var value {.test_greater $$var $value}
.set A $MAX_NESTING
.set MAX_NESTING 15
test_var_greater MAX_NESTING
test_var_greater MAX_NESTING 3 12
test_var_greater MAX_NESTING 3
.function_all_options ntimes n n {e can't have two parameters of the same name} 
.function_all_options ntimes n {
  .while test_var_greater n 0 {rwsh.mapped_argfunction {rwsh.argfunction}
                               .var_subtract n 1}}
w ntimes
ntimes -- 3 {e $n remaining $nl}
ntimes 3 -- {e -- must preceed all positional arguments}
ntimes 2 {ntimes 3 {e &&n and $n remaining $nl}}
.set MAX_NESTING $A
.function_all_options a [-x] [-] [--long-opt y second {
  e illegal missing close bracket}
.function_all_options a [-x] [--long-opt y] second {
  for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a single
a -x single
a --long-opt arg single
a --long-opt single
a --long-opt first -x --long-opt second single
.function_all_options a [-q option1 option2] [-x o1 o2 o3 o4] required {
  for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a single
a -q one two
a -q opt and req
a -x opt and req
a -x first second third fourth req
a -q one two -q three four five
a -x one two three four -q five six seven
.function_all_options a [-x] [-] [--long-opt] [-] y second {
  e illegal duplicate flag parameter}
.function_all_options a [-x] [-] [-x] [--long-opt] y second {
  e another illegal duplicate flag parameter}
.function_all_options a [-x] [-] [--long-opt] y second y {
  e illegal duplicate required parameter}
.function_all_options a [-x] [-] [--long-opt] -x second {
  e illegal duplication between flags and required parameters}
.function_all_options a [-] [--long-opt] -x second [-x] {
  e evil duplication between flags and required parameters}
.function_all_options a [-x] [-] [--long-opt] -- second {
  e -- cannot be a required parameter even if only an implicit option}
.function_all_options a -y [second] {
  for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a 1
a 1 2
a 1 2 3
.function_all_options a [-x] [-] [--long-opt] -y second {
  for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a 
a --long-opt -xx over-long flag
a -xx --long-opt over-long flag
a --long-op short flag
a - --long-op short flag
a no flags
a deficient
a flagless excess argument
a -x with flag
a -x -x doubled flag
a unaccepted -x interlaced_flag
a -x -- - flag_made_fixed_argument
a -- - flag_again_made_fixed_argument
a -x with excess argument
a - with flag
a --long-opt with flag
a -x - some_flags in_order
a - -x some_flags reversed
a - --long-opt some_flags in_order
a --long-opt - some_flags reversed
a -x --long-opt some_flags in_order
a --long-opt -x some_flags reversed
a -x - --long-opt deficient
a -x - --long-opt all_flags in_order
a --long-opt - -x all_flags reversed
a - --long-opt -x all_flags shuffled
a - --long-opt -x - -x some_flags doubled
a - -x - -x - one_doubled one_tripled
a --long-opt - -x -x - --long-opt all_flags doubled
.function_all_options a [-first] [-to] [--] {
  for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a 
a
a excess
a -to
a -first --
a -first excess
a -to -- -first
a -to -first
.function_some_flags a [-first] {
  for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a 
a
a excess
a -to
a -first --
a -first excess
a -to -- -first
a -to -first
.function_some_flags a {
  for &&{.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a 
a

# .global .local .unset .var_exists 
.global
.global x y z
.global x y {excess argfunc}
.local
.local x y z
.local x y {excess argfunc}
.unset
.unset x {excess argfunc}
.unset x y
.var_exists
.var_exists x {excess argfunc}
.var_exists x y
.global 100 nihilism
.unset #
.unset *
.unset 1
.unset MAX_NESTING
.var_exists #
.var_exists *
.var_exists 0
.var_exists 2
.var_exists x
.unset x
.global x nihilism
.var_exists x
e $x
.global x nihilism
.function a {if_only .var_exists x {e in a x \( $x \) $nl}
             if_only .var_exists y {e in a y \( $y \) $nl}
             .local x (first level not global)
             .local y (level one not global)
             b
             if_only .var_exists x {e out a x \( $x \) $nl}
             if_only .var_exists y {e out a y \( $y \) $nl}}
.function b {if_only .var_exists x {e in b x \( $x \) $nl}
             if_only .var_exists y {e in b y \( $y \) $nl}
             .local x (second level masks first)
             .set y   (level two overwrites one)
             c
             if_only .var_exists x {e out b x \( $x \) $nl}
             if_only .var_exists y {e out b y \( $y \) $nl}}
.function c {if_only .var_exists x {e in c x \( $x \) $nl}
             if_only .var_exists y {e in c y \( $y \) $nl}
             # can unset a local, but only one at a time
             .unset x
             .global y (attempting to create global masked by local)
             e $? $nl
             .set x (third level overwrites first)
             .local x (third level masks first)
             .set y (level three overwrites one)
             if_only .var_exists x {e out c x \( $x \) $nl}
             if_only .var_exists y {e out c y \( $y \) $nl}}
a
# demonstrating that final values are not retained
a
e $x
.var_exists y
.unset x
.var_exists x
.return 0

# .store_output
.store_output x
.store_output {e some text}
.store_output x {e some text}
.global x ()
.store_output x {e some text; .return 1}
e $x
.store_output x {e some text}
e $x
.unset x

# .if .else_if .else_if_not .else
.if
.else_if
.else_if_not
.else
.if missing argfunction
.else_if missing argfunction
.else_if_not missing argfunction
.if .return 0 {e if true; .return 1}
.else {e else true; .return 3}
.if .return 1 {e if false; .return 4}
.else {e else false; .return 5}
.else_if .return 0 {e not this one; .return 6}
.else_if_not .return 1 {e not this one; .return 7}
.if .return 1 {e nor this; .return 8}
.else_if .return 1 {e nor this; .return 9}
.else_if .return 0 {e but this; .return 10}
.else_if .return 0 {e this should be skipped; .return 11}
.else_if .return 1 {e and certainly this; .return 12}
.else {e nor this; .return 13}
.if .return 1 {e nor this; .return 14}
.else_if_not .return 0 {e nor this; .return 15}
.else_if_not .return 1 {e but this; .return 16}
.else_if_not .return 1 {e this should be skipped; .return 17}
.else_if_not .return 0 {e and certainly this; .return 18}
.else {e nor this; .return 19}
.if .return 0 {.if .return 1 {
	   not to be printed; .return 20}
       .else_if .return 0 {
	   e nested else_if; .return 21}}
.else {e else_if failed to appropriately set IF_TEST on exit; .return 22}
.if .nop {e set up}
.if .nop {e if before else}
.if .return 0 {.else_if .return 0 {e nested syntax; .return 23}}
.else {e already tested; .return 24}

# .internal_features .internal_functions .internal_vars
.internal_features 1
.internal_functions 1
.internal_functions {excess argfunc}
.internal_vars 1
.internal_features
.internal_functions
.internal_vars

# .is_default_input .is_default_output .is_default_error
.is_default_input 1
.is_default_input {excess argfunc}
m {.is_default_input <dummy_file}
m {.is_default_input}
.is_default_output 1
.is_default_output {excess argfunc}
e &{.is_default_output; .echo $?}
m {.is_default_output >dummy_file}
m {.is_default_output}
.is_default_error 1
.is_default_error {excess argfunc}
m {.is_default_error}

# .ls
.ls
.ls /bin {excess argfunc}
.ls /bin /usr/

# .nop
.nop
.nop {optional argfunc}
.nop 1 2 3 4 5

# .return
.return
.return 1 1
.return 0 {excess argfunc}
.return 0
.return 1
.return \
.return O
.return 1E2
.return 2147483647
.return 2147483649
.return -2147483649

# .selection_set
.selection_set A
.selection_set A /usr {excess argfunc}
.selection_set A /usr
e $A
.selection_set A ./
e $A
.selection_set A local/include
e $A
.selection_set A ..
e $A
.selection_set A ()
e $A
.selection_set A ./local/../../bin
e $A
.selection_set A sbin etc
e $A

# .set
.set A
.set B x
.set B x {excess argfunc}
.set IF_TEST x
.set A x
e $A

# .signal_handler
.signal_handler {.return A}
m {.signal_handler rwsh.not_a_number rwsh.executable_not_found {.return A}}
m {.signal_handler rwsh.not_a_number rwsh.executable_not_found {.eturn A}}
m {.signal_handler rwsh.not_a_number rwsh.executable_not_found {.echo A}}
m .echo hi {.signal_handler &{.internal_functions}$ {&&*}}

# .source
.source
.source /etc/hosts {excess argfunc}
.source /*fu*bar*
# if you actually have that file, something is seriously wrong
.source /etc/hosts
# .source is tested by having test_init.sh define #
w .init

# .stepwise
f wrapper {a $* two; a $* three}
f a {e $* one $nl; e $* two $nl; e $* three $nl}
f d {e $* $nl; .stepwise $* {d $*}}
.stepwise {e $* $nl}
.stepwise wrapper 1 2
.stepwise stepwise {e $* $nl}
.stepwise .stepwise {e $* $nl}
.stepwise wrapper 1 2 {e $* $nl}
f wrapper {a $* one; a $* two; a $* three}
wrapper 1 2
.stepwise wrapper 1 2 {d $*}
.stepwise wrapper 1 2 {e $* $nl}
.set A $MAX_NESTING
.set MAX_NESTING 15
.stepwise wrapper 1 2 {d $*}
.set MAX_NESTING $A 

# .test_string_equal .test_string_unequal .test_not_empty
.test_string_equal x
.test_string_equal x x x
.test_string_equal x x {excess argfunc}
.test_string_unequal x 
.test_string_unequal x x x
.test_string_unequal x x {excess argfunc}
.test_not_empty 
.test_not_empty x x
.test_not_empty x {excess argfunc}
.test_string_equal x y
.test_string_equal x x
.test_string_unequal x y
.test_string_unequal x x
.test_not_empty \
.test_not_empty x

# .test_is_number .test_number_equal .test_greater .test_less
.test_is_number
.test_is_number 42 {excess argfunc}
.test_is_number ()
.test_is_number 42a
.test_is_number 42
.test_is_number -0.314
.test_is_number 6.022e9000000000
.test_is_number 6.022e23
.test_number_equal 42
.test_number_equal 42 42 {excess argfunc}
.test_number_equal 42 42a
.test_number_equal 42b 42
.test_number_equal 0 0.000000000000000000000000001
.test_number_equal 1e-9000000000 1e-9000000001
.test_number_equal 0 0.0
.test_number_equal 42 42.01
.test_number_equal 42 42
.test_number_equal 6.022e9000000000 .6022e24
.test_number_equal 6.022e23 .6022e9000000001
.test_number_equal 6.022e23 6.022e2
.test_number_equal 6.022e23 .6022e24
.test_greater 6.022e23
.test_greater 6.022e23c 6.022e23 {excess argfunc}
.test_greater 6.022e23c 6.022e23
.test_greater 6.022e23 6.022e23e
.test_greater 6.022e9000000000 .6022e23
.test_greater 6.022e23 .6022e9000000001
.test_greater 6.022e2 6.022e23
.test_greater 6.022e23 .6022e24
.test_greater 6.022e23 6.022e2
.test_less 6.022e23
.test_less 6.022b23 6.022e23 {excess argfunc}
.test_less 6.022b23 6.022e23
.test_less 6.022e23 6.022a23
.test_less 6.022e9000000000 .6022e23
.test_less 6.022e23 .6022e9000000001
.test_less 6.022e23 6.022e2
.test_less 6.022e23 .6022e24
.test_less 6.022e2 6.022e23

# .which_executable .which_test .which_return
.which_test
.which_test j
.which_test #
.which_test rwsh.mapped_argfunction
.which_test rwsh.mapped_argfunction {rwsh.argfunction}
.which_executable
.which_executable j
.which_executable #
w rwsh.mapped_argfunction {.nop 1 () \ \\ \$ \@}
w rwsh.mapped_argfunction {.nop 1 () \  \\ \$ \@}
w rwsh.mapped_argfunction {$A $$A $0 $$$1 $# $* $*2 $A$ $$*$ $$$*12$}
w rwsh.mapped_argfunction {&&A &&0 &&* &&*3 &&$A$ &&*$ &&*6$}
w rwsh.mapped_argfunction {@a @$a @$1 @$* @$*2}
w rwsh.mapped_argfunction {>dummy_file}
.which_executable rwsh.mapped_argfunction
.which_executable rwsh.mapped_argfunction {rwsh.argfunction}
.waiting_for_shell j
.waiting_for_shell {excess argfunc}
# .waiting_for_shell
.waiting_for_user j
.waiting_for_user {excess argfunc}
# .waiting_for_user
.waiting_for_binary j
.waiting_for_binary {excess argfunc}
# .waiting_for_binary
.which_return
.which_return rwsh.mapped_argfunction
.which_return rwsh.mapped_argfunction {rwsh.argfunction}
.which_return .which_return
.which_return j
.which_return .waiting_for_binary

# .usleep .which_execution_count .which_last_execution_time
# .which_total_execution_time
.usleep
.usleep 8000 {excess argfunc}
.usleep -6
.usleep 5i
.usleep 8000
.which_execution_count
.which_execution_count j
.which_execution_count .usleep
.which_last_execution_time
.which_last_execution_time j
# .which_last_execution_time .usleep
.which_total_execution_time
.which_total_execution_time j
# .which_total_execution_time .usleep

# .which_path
.which_path cat
.which_path cat /bin {excess argfunc}
.which_path cat \
.which_path does<not>exist /bin:/usr/bin
.which_path cat /bin:/usr/bin

# .while
.function tf {.test_string_unequal $A $N}
.set A 0
.set N 4
.while {e ARGS}
.while tf
.while tf {e printed; .set A 4}
.while tf {e skipped}
.set A 0
.while tf {e in .while argfunction $A $nl; .var_add A 1}
.set A 0
.while tf {.if .return $A {.set A 1}
           .else {.function tf {.return 1}}
           e in overwriting argfunction $nl}

# .var_add
.var_add
.var_add A 1 2
.var_add A 1 {excess argfunc}
.var_add B 1
.set A A
.var_add A 2 
.set A 1e309
.var_add A 2 
.set A -1e308
.var_add A A
.var_add A 1e309
.var_add A -1e308
.var_add A 1e308
.var_add A 1e308
e $A
.var_add A 1e308
.var_add A -1e308
e $A
.var_add A \
e $A

# .var_divide
.var_divide A
.var_divide A 1 2
.var_divide A 1 {excess argfunc}
.var_divide B 1
.set A A
.var_divide A 2 
.set A 1e3000000000
.var_divide A 2 
.set A 16.8
.var_divide A A
.var_divide A 1e3000000000
.var_divide A 0 
.var_divide A 4.2 
e $A
.set A 1.8e-20
.var_divide A 1e308
.set A 0
.var_divide A 1e308
e $A

# .var_subtract
.var_subtract
.var_subtract A 1 2
.var_subtract A 1 {excess argfunc}
.var_subtract B 1
.set A A
.var_subtract A 2 
.set A 3000000000
.var_subtract A 2 
.set A 1e308
.var_subtract A -1e308
e $A
.set A -2147483648
.var_subtract A A
.var_subtract A 3000000000
.var_subtract A -2147483648
.var_subtract A 2147483647
.var_subtract A 2147483647
e $A
.var_subtract A 2147483647
.var_subtract A -2147483648
e $A
.var_subtract A \
e $A

# .version .version_compatible
.version 1.0
.version {excess argfunc}
.version_compatible
.version_compatible 1.0 1.0
.version_compatible 1.0 {excess argfunc}
.version
.version_compatible 1.0
.version_compatible 0.3+

# binary test implicitly tests Old_argv_t
/bn/echo 1 2 3
/bin/echo 1 2 3

# internal functions 
# rwsh.after_command rwsh.raw_command rwsh.prompt
# all of these are used as part of the test itself. If this changes, then the 
# following tests will fail.
w rwsh.after_command
w rwsh.prompt
w rwsh.raw_command

# rwsh.arguments_for_argfunction rwsh.before_command rwsh.binary_not_found
w x {rwsh.escaped_argfunction me}
f rwsh.before_command {.echo $*0; .echo $nl}
/bn
f rwsh.before_command

# rwsh.autofunction
w z
f rwsh.autofunction {e $*0 $nl}
z 1 2 3
w z
f rwsh.autofunction {y $*0}
z 1 2 3
w z
f rwsh.autofunction {f $1 {e $* $nl}}
z 1 2 3
w z
f rwsh.autofunction

# rwsh.double_redirection
# m {e hi #>dummy_file >dummy_file}
# m {e hi >dummy_file}
# m {e &{e hi #>dummy_file}}

# rwsh.executable_not_found
m {m {x; e should not be printed}}
f rwsh.executable_not_found
w rwsh.executable_not_found
w x
x
w rwsh.executable_not_found

# rwsh.mapped_argfunction rwsh.unescaped_argfunction rwsh.argfunction
# rwsh.escaped_argfunction
rwsh.mapped_argfunction 1 2 3 {e a $* a}
rwsh.mapped_argfunction
f g {w rwsh.argfunction {rwsh.unescaped_argfunction}
     w rwsh.argfunction {rwsh.argfunction}
     w rwsh.argfunction {rwsh.escaped_argfunction}}
g {}

# rwsh.excessive_nesting
f g {h}
f h {g}
g
f rwsh.excessive_nesting {h}
g

# rwsh.run_logic
f rwsh.run_logic {.if .return $1 {.nop}; .else_if $*2 {.nop}; .else {.nop}}
0 e don't print
1 e do print
1 f rwsh.run_logic
1 e executable not found

# rwsh.undefined_variable
e $WRONG $WWRONG
e &WWRONG

# rwsh.vars
rwsh.vars

# check for extraneous variables
/usr/bin/printenv

# .importenv_preserve .importenv_overwrite
.global SHELL unmodified
.importenv_preserve x
.importenv_preserve {excess argfunc}
.importenv_preserve
e $TESTABILITY
e $SHELL
.unset TESTABILITY
.importenv_overwrite x
.importenv_overwrite {excess argfunc}
.importenv_overwrite
e $TESTABILITY
e $SHELL

# exiting rwsh.shutdown
.exit excess_argument
.exit {excess argfunction}
.exit
.echo 1 2 3
