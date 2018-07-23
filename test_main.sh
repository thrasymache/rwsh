/bin/cat /tmp/lcl
.nop .init is tested by having test_init.sh define rwsh.before_command etc.
.source /etc/rwshrc-basic
.function_all_flags ## [args ...] {.nop $args}

## argm
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
rwsh.mapped_argfunction {	   .echo ignore leading tab in argfunction}
.which_executable rwsh.mapped_argfunction {.nop}
.which_executable rwsh.argfunction {
  multiple line argfunction }
.nop rwsh.argfunction rwsh.mismatched_brace } 
.source test_files/unclosed_brace_newline.rwsh
.source test_files/unclosed_brace.rwsh
.source test_files/unclosed_parenthesis_newline.rwsh
.source test_files/unclosed_parenthesis.rwsh
.nop multiple statements \; on a line
.which_executable rwsh.argfunction {rwsh.multiple_argfunctions} {}
.which_executable rwsh.argfunction {rwsh.argfunction with text args}
.which_executable rwsh.argfunction {rwsh.argfunction {with a function arg}}
.which_executable rwsh.argfunction {{{{{{{{{{{}}}}}}}}}}}
.which_executable rwsh.argfunction {
  .function  x  { ${ .which_path  echo  $PATH }  something } }

## ability of functions to perform custom control flow
# rwshrc-basic
.function f {.function $1 {rwsh.argfunction}}
fni w command {.which_executable $command {rwsh.argfunction}}
f e {.echo $*}
f m {rwsh.argfunction}
.which_executable f
.which_executable w
.which_executable e
.which_executable m
which e
which ee
which () {}
e text that does not have a prompt appended
m {e again}
if_only .return 1 {e not printed}
if_only .return 0 {e printed without error}
.function for {
  .if .test_greater $# 1 {.for &&*1$ {rwsh.argfunction}}; .else {.nop}}
for {e skipped without error}
for 1 2 3 {e loop $* $nl}

## arg_script.cc and arg_spec.cc
# Arg_spec::FIXED, Arg_script::add_quote
e 5 4 3 2 1
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
e (internal \)parenthesis \\ escape ( \))) $#

# star_var (argm_star_var)
e 1 2 $* 3 4
e $*2 1 2

# star_soon
.if .nop 1 2 3 {e &*}
.else {}
.if .nop 1 2 3 {e &*0}
.else {}

# selection_read read_dir()
e @/etc
e @test_files/i*xx
e @test_files/i*xx/f*
e @/*selection_not_found*
e @test_main.cc
m {m {.for @e*c {e $1 $nl}} >test_files/tmp}
.global LC_ALL C
.nop $LC_ALL
f sort {&{.which_path sort /bin:/usr/bin} $*}
sort test_files/tmp
e @test_files/*xx
e @test_files/*x*x*x*x
e @test_files/*xyxy
e @/bin
m {m {.for @/usr/*bin {e $1 $nl}} >test_files/tmp}
sort test_files/tmp
m {m {.for @/etc/rwsh* {e $1 $nl}} >test_files/tmp}
sort test_files/tmp
e @/etc/rw*ic
e @/etc/rwsh*a*
.set FIGNORE *de*
e @/etc/rwsh*a*
e @test_files/*i*x*y*y*x*
e @test_main.sh
m {m {.for @test_files/*x* {e $1 $nl}} >test_files/tmp}
sort test_files/tmp
.scope r*h.cc sel*.h (A ...) {e @$A}

# Arg_spec::REFERENCE, interpret(), evaluate_expansion(), evaluate_var()
.set A /bin
.global B ( (zero   zero) ((one one  ) one   ) two three)
.global C ((external) () ( ) internal(parenthesis))
.global broken (extra_close\) \(extra_open)
e $A $0 @$A
e A $1 1 $$3 $$$3
e A 1 2 3 4 5 6 7 $$$$$$$$$8
m $FOO {}
m {e $FOO}
m $B$$1x {e $# $*}
m $B$$1$ {e $# $*}
m $B {e $# $*}
m $B$ {e $# $*}
m $B$$ {e $# $*}
m $B$$$$ {e $# $*}
m {m $B$$$$ {e $# $*}}
m $B$10 {e $# $*}
m $B$1 {e $# $*}
m $B$$1 {e $# $*}
m $broken {e $# $*}
m $broken$ {e $# $*}
m $broken$$ {e $# $*}
m $C {e $# $*}
m $C$ {e $# $*}
m $C$$ {e $# $*}

# Arg_spec::SOON, apply()
e A &1 1 &$3 &$$3
e &&A
m {e &&&A}
m {e &&&without_mismatched_brace
}
e &{e &&A}
e &&{e &A}
e &A
.scope not_bin A {e &A &&A $A $nl; .scope otherwise A {e &A &&A &&&A $A}}
m {m &B$10 {e $# $*}}
m {m &B$$$$ {e $# $*}}
m {m &B$1 {e $# $*}}
m {m &B$$1 {e $# $*}}

# Arg_spec::SUBSTITUTION and Arg_spec::SOON_SUBSTITUTION, apply(), interpret(),
# evaluate_substitution()
e ${e $A}
w rwsh.argfunction {e ${e $A}}
.scope not_bin A {
   e &{.echo $A} &&{.echo $A} $A $nl
   .scope otherwise A {
      e &{.echo $A} &&{.echo $A} &&&{.echo $A} ${.echo $A} $A}}
.scope not_bin A {e &{.echo &A $A} &&{.echo &A &&A} ${.echo &A $A}}
m &{.echo $A} {e $1 &1}
m &{.return 1} {}
m ${.return 1} {}
m {e &{.return 1}}
m {e &&{.return 1}; e after}
f rwsh.failed_substitution {e $Z}
.throw m {echo even from $* 7 is a number}
.fallback_handler m {echo even from $* 7 is a number}
.throw rwsh.failed_substitution m {echo even from $* 7 is a number}
m {e &&{.return 1}; e after}
f rwsh.failed_substitution {.echo signal triggered: $0 \( $* \) $nl; .return -1}
e x{e bad argfunction style}
e x&&&{e x}
e $+{e x}
e &+{e x}
e &&${e x}
.return &{.return 0}
.return ${e 0 $nl}
.return &{.echo 0}
e nevermore &{/bin/echo quoth the raven} 
m ${e $B}@ {e $# $*}
m ${e $B} {e $# $*}
m &{e $B} {e $# $*}
m ${e $B}$ {e $# $*}
m &{e $B}$ {e $# $*}
e $# &{e $B}$
m {e $# &{e $B}$}
m &{e $B}$$ {e $# $*}
m ${e $B}$$ {e $# $*}
m {m ${e $B}$1 {e $# $*}}
m {m &{e $B}$1 {e $# $*}}
m {m ${e $B}$$$1 {e $# $*}}
m {m &{e $B}$$$1 {e $# $*}}
.unset A
.unset B
m &{e ((zero zero) (one one) two three)}$10 {e $# $*}
m ${e (zero zero) \)one one two three}$1 {e $# $*}
m &{e (zero zero) \(one one two three}$1 {e $# $*}
m ${e (zero zero) \)one one two three} {e $# $*}
c x &{.echo (y y)}$ x
c x ${.echo ( y y )}$ x
c x &{.echo (( y) (y ))}$ x
c x ${.echo (    )}$ x
c x &{.echo (
y
y
)}$ x

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
.global OLD_NESTING ${.get_max_nesting}
.set_max_nesting 46
f x {.var_add A 1
     m {.var_add A 1
        m {.var_add A 1
           m {.var_add A 1
              m {.var_add A 1
                 m {rwsh.argfunction}}}}}}
.scope 00 A {x {e &A &&A &&&A $A}}
.scope 00 A {x {x {x {x {e &A &&A &&&A &&&&A &&&&&A &&&&&&A $A}}}}}
.scope 00 A {
  x {e &{.echo &A $A} . &&{.echo &A &&A $A} . &&&{.echo &A &&A &&&A $A} . ${
  .echo &A &&A &&&A $A} . $A}}
.set A 0
x {x {x {x {
  e &{.echo &A $A} . &&{.echo &A &&A $A} . &&&{.echo &A &&A &&&A $A} . &&&&{
  .echo &A &&A &&&A &&&&A $A} . &&&&&{.echo &A &&A &&&A &&&&A &&&&&A $A} . ${
  .echo &A &&A &&&A &&&&A &&&&&A $A} . $A}}}}
f x
.set_max_nesting $OLD_NESTING
.unset A
.unset OLD_NESTING

## builtin tests
# .argc
.argc {excess argfunc}
.argc
.argc ()
.argc (1 a) (2 b (c d)) (3 ((e () f)))

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

# .fallback_handler .get_fallback_message .set_fallback_message
.fallback_handler
.fallback_handler something {excess argfunc}
.get_fallback_message something 
.get_fallback_message {excess argfunc}
.set_fallback_message
.set_fallback_message something {excess argfunc}
.fallback_handler pretend_error
.local original_message &{.get_fallback_message}
.set_fallback_message spaceless fallback message
.fallback_handler second pretend error
.set_fallback_message (alternate fallback message: )
.fallback_handler a third
.set_fallback_message $original_message

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

# .function_all_flags
.function_all_flags
.function_all_flags /bin/echo {e cannot define a path to be a function}
.function_all_flags .exit {e cannot redefine a builtin as a function}
.function_all_flags .a {can define a function for non-existant builtin}
.function_all_flags rwsh.argfunction {e cannot define rwsh.argfunction}
.function_all_flags a y y {e illegal duplicate required parameter}
.function_all_flags a [-x] [-x] {e illegal duplicate flag parameter}
.function_all_flags a [x x] {e illegal duplicate optional parameter}
.function_all_flags a [-x arg bar] [-y arg] {e illegal duplicate flag argument}
.function_all_flags a -x [-x] {e evil duplication between flags positional}
.function_all_flags a -- -- {e -- cannot be a duplicate parameter}
.function_all_flags a [--] [--] {e [--] cannot be a duplicate parameter}
.function_all_flags a [--] -- {e -- and [--] cannot both be parameters}
.function_all_flags a [-- arg] {e -- cannot take arguments}
.function_all_flags a [arg -- foo] {e -- cannot take arguments}
.function_all_flags nonsense
w test_var_greater
.scope 5 n {test_var_greater n}
.scope 5 n {test_var_greater n 3 12}
.scope 5 n {test_var_greater n 3}
w ntimes
ntimes -- 3 {e $n remaining $nl}
ntimes 2 {ntimes 3 {e &&n and $n remaining $nl}}
.function_all_flags a [-x] [-] [--long-opt y second {
  e mismatched bracket (i.e. missing close brakcet)}
.function_all_flags a [-?] [--] {.list_locals}
.function_all_flags a [-x] [--] foo {.list_locals}
.function_all_flags a [-?] -- foo {.list_locals}
.function_all_flags a [-x] -- {.list_locals}
.function_all_flags a [--] {.list_locals}
w a
a
a --
a foo
.function_all_flags a [-x] [--long-opt y] second {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a single
a -x single
a --long-opt arg single
a --long-opt single
a --long-opt first -x --long-opt second single
.function_all_flags a [-q option1 option2] [-x o1 o2 o3 o4] required {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a single
a -q one two
a -q opt and req
a -x opt and
a -x first second third fourth req
a -q one two -q three four five
a -x one two three four -q five six seven
a -x one two three four five -q six seven
a one -x two three four five -q six seven
.function_all_flags a [optional0] -- [optional1 optional2 optional3] required {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a single
a one two
a one two three
a one two three four five
a one two three four five six seven eight nine
.function_all_flags a -y [second] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a 1
a 1 2
a 1 2 3
.function_all_flags a [-x] -y [--long-opt] second [--] [-] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a --long-opt -xx over-long flag
a -xx --long-opt over-long flag
a --long-op short flag
a - --long-op short flag
a no flags
a deficient
a flagless but_has excess argument
a -x with flag
a -x -x doubled flag
a accepted -x interlaced_flag -
a tardy flags_last - -x
a -x -- - flag_made_fixed_argument
a -- - flag_again_made_fixed_argument
a -x -- flag_and_fixed-x -x
a -x just flags-x --long-opt
a -x just flags-x -- --long-opt
a -x just flags-x --other
a -x just flags-x -- --other
a -x with one excess argument
a - with flag
a --long-opt with flag
a --long-opt -x - some_flags in_order
a - -x --long-opt some_flags reversed
a - --long-opt some_flags in_order
a --long-opt - some_flags reversed
a -x --long-opt some_flags in_order
a --long-opt -x some_flags reversed
a -x --long-opt deficient
a -x - --long-opt all_flags in_order
a --long-opt - -x all_flags reversed
a - --long-opt -x all_flags shuffled
a - --long-opt -x -x some_flags doubled
a -x --long-opt -x --long-opt -x one_doubled one_tripled
a --long-opt -x -x - --long-opt all_flags doubled
.function_all_flags a [-first] [-to] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a
a
a excess
a -to
a -first --
a -first excess
a -to -- -first -- stops flag parsing rather than being a flag
a -to -first
.function_all_flags a [-?] [-first] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a
a
a excess
a -to
a -first --
a -first excess
a -to -- -first
a -to -first
.function_all_flags a [-*] [-first] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a
a
a -to -- -first
a -to -first
.function_all_flags a [-?] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a
a
.function_all_flags a ... y {}
.function_all_flags a [-x] [--file file] ... y {}
.function_all_flags a [...] x {}
.function_all_flags a [... y] x {}
.function_all_flags a x ... [y z] {}
.function_all_flags a x [... y] z ... {}
.function_all_flags a [y z] ... x {}
.function_all_flags a [y] ... x {}
w a
.function_all_flags a x ... y ... {}
.function_all_flags a [x ...] [y z] {}
.function_all_flags a [x ...] y ... {}
.function_all_flags a [x ... a] [y] {}
.function_all_flags a [x ... a] y ... {}
.function_all_flags a [x ... a ...] {}
.function_all_flags a [x ... a ... b] {}
.function_all_flags a [-x ...] b [c] {}
.function_all_flags a -? x ... y {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a -c -a -b first second third
a -c first -a second -b third
.function_all_flags a -* x ... y {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a -c -a -b first second third
a -c first -a second -b third
.function_all_flags a [-?] x ... y {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a 
a first
a first (se cond)
a first (se cond) third
a first (se cond) third fourth (fi fth)
.function_all_flags a [-?] x [...] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a first second third fourth fifth
.function_all_flags a [-?] x [--] [y ...] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a first
a first second
a first second third
a first second third fourth fifth
.function_all_flags a [-?] a [b ... c] d {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a first
a first second
a first second third
a first second third fourth
a first second third fourth fifth
a first second third fourth fifth sixth
.function_all_flags a [-x ...] b c {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a first second
a -x first
a -x first second
a -x first second third
a -x first second third fourth
.function_all_flags a [-x b ...] c {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x first -x (sec ond) third
a -x first -y (sec ond) third
a -x (fi rst) (sec ond) third fourth
.function_all_flags a [-x ... b] c {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  .combine $nl
  if_only .test_not_empty $-* {c (-*: ) $-*$ $nl}
  if_only .var_exists -x {c (-x: ) $-x$ $nl}
  if_only .var_exists b {c (b: ) $b$ $nl}
  if_only .var_exists c {c (c: ) $c$ $nl}
}
w a
a first
a -x first
a -x (fi rst) second
a -x () (fi rst) second
a -x first (sec ond) third
a -x (fi rst) (sec ond) third fourth
.function_all_flags a x [-?] [... y z] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  .combine $nl
  if_only .var_exists x {c (x: ) $x$ $nl}}
w a
a
a (fi rst)
a (fi rst) second
a first (sec ond) third
a (fi rst) (sec ond) third fourth
a () (sec ond) third fourth
a (fi rst) (sec ond) (thi rd) (fou rth) (fi fth)
.function_all_flags a [-?] [x] [... y] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a (fi rst)
a (fi rst) second
a first (sec ond) third
a (fi rst) (sec ond) third fourth
a (fi rst) (sec ond) (thi rd) (fou rth) (fi fth)
.function_all_flags a [-?] [x y] [... z] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  .combine $nl
  if_only .var_exists x {c (x: ) $x $nl}
  if_only .var_exists y {c (y: ) $y$ $nl}
  if_only .var_exists z {c (z: ) $z$ $nl}}
w a
a
a (fi rst)
a (fi rst) second
a (fi rst) (sec ond) (thi rd)
a (fi rst) (sec ond) third (fou rth)
a (fi rst) (sec ond) (thi rd) (fou rth) (fi fth)
.function_all_flags a -- [-x] y z {}
.function_all_flags a -- [x y] z {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
a
a --file
a first second third
a -x second -

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
.global 100 nihilism
.global .var_exists (must be requested to be checked)
e $.var_exists
.unset #
.unset #
.unset *
.unset 1
.unset MAX_NESTING
.var_exists #
.var_exists *
.var_exists 0
.var_exists 2
.var_exists x
.var_exists x y
.var_exists # y
.var_exists y x
.unset x
.global x nihilism
.var_exists x
.var_exists x y
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

# if_core
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
.if .nop {e set up}
.if .nop {e if before else}
.if .return 0 {.else_if .return 0 {e nested syntax; .return 20}}
.else {e already tested; .return 21}
.if .return 0 {.if .return 1 {
	   not to be printed; .return 22}
       .else_if .return 0 {
	   e nested else_if; .return 23}
       .else {
	   e nested else not printed; .return 24}}
.else {e still not printed; .return 25}
.if .return 0 {.if .return 1 {
	   not to be printed; .return 22}
       .else_if .return 0 {
	   e about to trigger bad_if_nest $nl; .return 23}}
.else {e else_if failed to appropriately set IF_TEST on exit; .return 24}
.if .return 1 {}
.else {
  .if .return 1 {e about to trigger bad_if_nest $nl}}
.if .return 0 {.if .return 0 {
	   /bin/false should nest properly; .return 22}
       .else {
	   e nested else not printed; .return 24}}

# .if .else_if .else_if_not .else
.if
.else_if .return 0 {e do not run after an exception}
.else_if_not .return 1 {e do not run after an exception}
.else_if
.else_if_not
.else {e do not run after an exception}
.if missing argfunction
.else_if missing argfunction
.else_if_not missing argfunction
.else
.if .return 1 {}
.else
.else {e second else for if}
.if .return 0 {e if true; .return 1}
.else {e else true; .return 3}
.if .return 1 {e if false; .return 4}
.else {e else false; .return 5}
.else {}
.else_if .return 0 {e not this one; .return 6}
.else {}
.else_if_not .return 1 {e not this one; .return 7}
.else {}

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
e ${.is_default_output; .echo $?}
m {.is_default_output >dummy_file}
m {.is_default_output}
.is_default_error 1
.is_default_error {excess argfunc}
m {.is_default_error}

# .list_executables
.list_executables excess
.list_executables {excess argfunc}
.for ${.list_executables}$ {.combine $1 $nl}

# .ls
.ls
.ls /bin {excess argfunc}
.ls /bin /usr/

# .nop
.nop
.nop {optional argfunc}
.nop 1 2 3 4 5

# .getpid .getppid
.getpid excess
.getpid {excess argfunc}
.getppid excess
.getppid {excess argfunc}
.fork m {/bin/kill ${.getppid}}
.fork m {/bin/kill ${.getpid}}
m {.fork m {/bin/kill ${.getppid}
            echo after the signal in subshell}
   echo after the signal in parent}
m {.fork m {/bin/kill ${.getpid}
            echo after the signal in subshell}
   echo after the signal in parent}

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

# .scope
.scope {e $foo}
.scope foo
.scope a (y y) {e illegal duplicate required parameter}
.scope a ([-x] [-x]) {e illegal duplicate flag parameter}
.scope a ([x x]) {e illegal duplicate optional parameter}
.scope a ([-x arg bar] [-y arg]) {e illegal duplicate flag argument}
.scope a (-x [-x]) {e evil duplication between flags positional}
.scope -- -- {e -- as a prototype forbids even -- as arguments}
.scope -- () {e but ${.list_locals} is acceptable for empty prototype}
.scope a (-- --) {e -- cannot be a duplicate parameter}
.scope a ([--] [--]) {e [--] cannot be a duplicate flag parameter}
.scope a ([--] --) {e -- and [--] cannot both be parameters}
.scope a ([-- arg]) {e -- cannot take arguments}
.scope a ([arg -- foo]) {e -- cannot be an argument}
.scope -x -y a b ([-?] args ...) {for ${.list_locals}$ {.combine $1 = $$1 \ }}
.scope a ([-? bad] arg) {e -? cannot currently take arguments}
.scope a ([-* bad] arg) {e -* (aka -?) cannot currently take arguments}
.scope -a -* -b a ([-?] a) {for ${.list_locals}$ {.combine $1 = $$1 \ }}
.scope bar foo {e aa $foo bb}
.scope baz bax (foo bar) {for ${.list_locals}$ {.combine $1 = $$1 \ }}
.scope foo bar baz bax (args ...) {e aa $args$2 bb $args$1 cc}
.scope single ([-x] [--long-opt y] second) {var_val ${.list_locals}$}
.function_all_flags a [-x] [--long-opt y] second {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
w a
.function_all_flags pt -- args ... {
  .scope $args$ ([-x] [--long-opt y] second) {
    for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
  .combine $nl
  .scope $args$ ( [-?] [--long-opt y] second) {
    for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}}
w pt
a
a single
pt single
a -x single
pt -x single
a --long-opt arg single
pt --long-opt arg single
a --long-opt single
pt --long-opt single
a --long-opt first -x --long-opt second single
pt --long-opt first -x --long-opt second single
.function_all_flags a [-?] [-first] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
w a
.function_all_flags pts -- [args ...] {
  .if var_exists args {
    .scope $args$ ([-first] [-?]) {
      .for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
      e nothing_required}
  else {.scope ([-first] [-?]) {
      .for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
      e nothing_required}}
w pts
a
pts
a excess
pts excess
a -to
pts -to
a -first --
pts -first --
a -first excess
pts -first excess
a -to -- -first
pts -to -- -first
a -to -first
pts -to -first
.return 0

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
.set 1 x
.set B x
.set B x {excess argfunc}
.set A x
e $A

# .get_max_collectible_exceptions .set_max_collectible_exceptions
.get_max_collectible_exceptions excess
.set_max_collectible_exceptions
.get_max_collectible_exceptions {excess}
.set_max_collectible_exceptions 1 {excess}
.set_max_collectible_exceptions NaN
.set_max_collectible_exceptions 0
.get_max_collectible_exceptions
.set_max_collectible_exceptions 7
.collect_errors_except
.collect_errors_only
m {.collect_errors_except .nop {
     m {echo before exception
        .throw rwsh.not_a_number 7
        echo after exception}
     echo between exceptions
     .scope () {.throw rwsh.executable_not_found foo}
     echo inside collect}
   echo outside collect}
m {.collect_errors_except echo {
     .throw rwsh.executable_not_found foo
     echo between exceptions
     .throw echo 7
     echo inside collect}
   echo outside collect}
# .collect_errors_except .echo {${.throw .echo exception thrown directly}}
m {.collect_errors_only rwsh.executable_not_found {
     .throw rwsh.executable_not_found foo
     echo between exceptions
     .throw echo 7
     echo inside collect}
   echo outside collect}
.collect_errors_except echo {
 .collect_errors_except echo {
  .collect_errors_except echo {
   .collect_errors_except echo {
    .collect_errors_except .echo {
     echo inside 5
     .throw echo inside 5
     .collect_errors_except echo {
      .collect_errors_except echo {
       .collect_errors_except echo {
        .collect_errors_except echo {
         .collect_errors_except .echo {
          echo inside 10
          .throw echo inside 10
          .collect_errors_except echo {
           .collect_errors_except echo {
            .collect_errors_except echo {
             .collect_errors_except echo {
              .collect_errors_except .echo {
               echo inside 15
               .throw echo inside 15
               .collect_errors_except echo {
                .collect_errors_except echo {
                 .collect_errors_except echo {
                  .collect_errors_except echo {
                   .collect_errors_except .echo {
                    echo inside 20
                    .throw echo inside 20
                   .collect_errors_except .echo {echo inside 21}
}}}}}}}}}}}}}}}}}}}}
.set_max_collectible_exceptions 1
.function_all_flags collecting_handler args ... {
  echo $args$
}
.set_max_extra_exceptions 1
.try_catch_recursive collecting_handler {
  .collect_errors_except .echo {
    .throw collecting_handler 1
    .throw collecting_handler 2}}
.try_catch_recursive collecting_handler {
  .collect_errors_except .echo {
    .throw collecting_handler 1
    .throw collecting_handler 2
    echo almost nothing}}
.set_max_extra_exceptions 0
.try_catch_recursive collecting_handler {
  .collect_errors_except .echo {
    .throw collecting_handler 1
    .throw collecting_handler 2}}
.try_catch_recursive collecting_handler {
  .collect_errors_except .echo {
    .throw collecting_handler 1
    .throw collecting_handler 2
    echo almost nothing}}
.set_max_extra_exceptions 1
.get_max_collectible_exceptions

# .try_catch_recursive .get_max_extra_exceptions .set_max_extra_exceptions
.try_catch_recursive rwsh.executable_not_found 
.try_catch_recursive {.return A}
f e_after {m {rwsh.argfunction}; echo after}
e_after {.try_catch_recursive rwsh.not_a_number rwsh.executable_not_found {
  .return A}}
f rwsh.autofunction {.nop}
e_after {.try_catch_recursive rwsh.executable_not_found {.eturn A}}
e_after {.try_catch_recursive rwsh.not_a_number {.return A}}
e_after {.try_catch_recursive rwsh.not_a_number {.cho A}}
e_after {.try_catch_recursive rwsh.not_a_number rwsh.executable_not_found {echo A}}
e_after {m echo hi {.try_catch_recursive ${.internal_functions}$ {&&&*}}}
.get_max_extra_exceptions excess
.set_max_extra_exceptions
.get_max_extra_exceptions {excess}
.set_max_extra_exceptions 1 {excess}
.set_max_extra_exceptions NaN
.set_max_extra_exceptions -1
.get_max_extra_exceptions
.set_max_extra_exceptions 0
.get_max_extra_exceptions
e_after {.try_catch_recursive rwsh.not_a_number {.try_catch_recursive rwsh.not_a_number {.return A}}}
e_after {.try_catch_recursive rwsh.not_a_number {.try_catch_recursive rwsh.not_a_number {.return A}}}
e_after {m echo hi {.try_catch_recursive ${.internal_functions}$ {&&&*}}}

# .source
.source
.source /etc/hosts {excess argfunc}
.source test_files/*fu*bar*
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

# .enable_readline .disable_readline .toggle_readline
.toggle_readline excess
.toggle_readline {excess}
.enable_readline excess
.enable_readline {excess}
.disable_readline excess
.disable_readline {excess}
.return 0
.nop .enable_readline
.nop .disable_readline
.nop .toggle_readline
.nop .toggle_readline

# .which_executable .which_test .which_return
# Arg_script::str() but only an unknown fraction of the lines
# Arg_spec::str() (except trailing whitespace) only through SOON case
.which_test
.which_test j
.which_test #
.which_test rwsh.mapped_argfunction
.which_test rwsh.mapped_argfunction {rwsh.argfunction}
.which_executable
.which_executable j
.which_executable #
w rwsh.mapped_argfunction {.nop 1 () \ \\ \$ \@ \) \(}
w rwsh.mapped_argfunction {.nop 1 () \  \\ \$ \@ \) \(}
w rwsh.mapped_argfunction {@a @$a @$1 @$* @$*2}
w rwsh.mapped_argfunction {>dummy_file}
# new tests here
w rwsh.mapped_argfunction {{&&{&&x &&{e}}$$$ ${&&x ${e}}$$$ {&&&x &&&{e} {&&&&x &&&&{e}}}}}
w rwsh.mapped_argfunction {$A $$A $0 $$$1 $# $* $*2 $A$$$ $A$10 $$*$ $$$*12$}
w rwsh.mapped_argfunction {&&A &&0 &&* &&*3 &&$A$$$ &&$A$10 &&*$ &&*6$ {&&&A$ &&&A$10}}
.function_all_flags wm [args ...] {
   w rwsh.mapped_argfunction {rwsh.argfunction}
   .echo $nl
   m $args$ {rwsh.argfunction}}
wm (aa ab ac) bb cc {e x &&1 &2 $3 y &&1$1 z &&1$2; m &&1$ {e () w $* $#}}
m (aa ab ac) bb cc {m $* {
  m &1 {echo $* $#}
  m &&1 {echo $* $#}
  m &&&1 {echo $* $#}
  m $1 {echo $* $#}
  m $1$ {echo $* $#}}}
wm () {.echo \\\ \ \\\ \\\\ \ \\|\\\ \  \  \\ \( \) \\\)}
wm () {.echo (\\  \\ \\\\) ( \\|\\  ) ( ) (\\) (\() (\)) (\\\))}
wm () {.echo a\\bc\\d\\\\e fg\\|\\hij klm \\ \) \( \\\(}
wm () {
.echo \( \) \\( \\) \\\( \\\) \\\\( \\\\) \\\\\( \\\\\)}
wm () {
.echo \) \( \\) \\( \\\) \\\( \\\\) \\\\( \\\\\) \\\\\(}
wm () {.echo () xx(yy \$ \@ \$$ \@@ }
wm () {.echo (  ) (	) (
)}
w rwsh.mapped_argfunction {.nop 1}
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
.which_return rwsh.mapped_argfunction {echo not tracked}
.which_return j
.which_return .which_return
.which_return .waiting_for_binary

# .which_last_exception
.which_last_exception .which_last_exception
.which_last_exception
.which_last_exception .try_catch_recursive {excess argfunc}
.which_last_exception .which_last_exception
.which_last_exception .which_last_return
.which_last_exception .try_catch_recursive
.which_last_exception .which_last_execution_time
.which_last_exception /bin/which
.which_last_exception /bin/cat
.which_last_exception /bin/echo
.which_last_exception e
.which_last_exception test_var_greater

# .usleep .which_execution_count .which_last_execution_time
# .which_total_execution_time
.usleep_overhead excess
.usleep_overhead {excess argfunc}
.usleep_overhead 
.usleep
.usleep 800 {excess argfunc}
.usleep -6
.usleep 5i
.usleep 800
# .usleep_overhead 
.which_execution_count
.which_execution_count j
.which_execution_count rwsh.mapped_argfunction {echo not tracked}
.which_execution_count .usleep
.which_last_execution_time
.which_last_execution_time j
.which_last_execution_time rwsh.mapped_argfunction {echo not tracked}
# .which_last_execution_time .usleep
.which_total_execution_time
.which_total_execution_time rwsh.mapped_argfunction {echo not tracked}
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
.while .return 0 {.throw .echo exception within while}
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
.set A -1e308
.var_subtract A 1e308
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
.which_return /bn/echo
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
.which_return /bn
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

# rwsh.excessive_nesting Base_executable::exception_handler
f g {h}
f h {g}
g
.stepwise g {e $* $nl; $*}
f rwsh.excessive_nesting {h}
g
f rwsh.excessive_nesting {e &&{.return 1}}
f rwsh.failed_substitution {e $Z}
g
e_after {.try_catch_recursive rwsh.undefined_variable rwsh.excessive_nesting rwsh.failed_substitution {g}}
.set_max_extra_exceptions 5
e_after {.try_catch_recursive rwsh.undefined_variable rwsh.excessive_nesting rwsh.failed_substitution {g}}
e_after {.try_catch_recursive rwsh.undefined_variable rwsh.failed_substitution {
  e ${.return 1}}}
f rwsh.else_without_if {e ${.return 1}}
e_after {.try_catch_recursive rwsh.undefined_variable rwsh.else_without_if {
  .else {}}}
f rwsh.failed_substitution {e ${.return 2}}
e_after {.try_catch_recursive rwsh.undefined_variable rwsh.failed_substitution {
  e ${.return 1}}}
f rwsh.else_without_if {e $Z}
e_after {.try_catch_recursive rwsh.undefined_variable rwsh.else_without_if {
  .else {}}}
f rwsh.failed_substitution {.return Z}
e_after {.try_catch_recursive rwsh.not_a_number rwsh.failed_substitution {
  e ${.return 1}}}

# .throw
.throw
.throw .nop
.throw rwsh.not_a_number 7
m {if_only .return 0 {.throw m {echo even from $* 7 is a number}}}
m {.try_catch_recursive echo {
      echo first
      .throw echo failing successfully
      echo second}
   echo third}

# rwsh.run_logic
f rwsh.run_logic {.if .return $1 {.nop}; .else_if $*2 {.nop}; .else {.nop}}
0 e don't print nuthin'
1 e do print
1 f rwsh.run_logic
1 e executable not found

# rwsh.undefined_variable
e $WRONG $WWRONG
e &WWRONG

# rwsh.vars
rwsh.vars

# check for extraneous variables and that export_env doesn't bless
.scope M val (N necessary) {/usr/bin/printenv; .nop $necessary}

# .list_environment
.global SHELL unmodified
.list_environment x
.list_environment {excess argfunc}
for ${.list_environment}$ {
  .scope $1$ (var val) {
    .global $var $val}}
e $TESTABILITY
e $SHELL
.unset TESTABILITY
for ${.list_environment}$ {
  .scope $1$ (var val) {setf $var $val; .nop $$var}}
e $TESTABILITY
e $SHELL
/usr/bin/printenv
.set_max_collectible_exceptions 7
.collect_errors_except .nop {${.throw echo exception from inside substitution}}
.collect_errors_only .nop {${.throw echo exception from inside substitution}}

## unused variables
# bless_unused_variables
.scope () {}
.scope -- () {}
.scope -- () {echo $--}
.scope -- (-- [a]) {}
.scope () {.local a n; .local b o; .local c p}
.scope () {
  .local a n; .local b o; .local c p
  echo $a $b $c $d
  .local d too_late}
.scope () {
  .local a n; .local b o; .local c p
  echo $d $a $b $c
  .local d too_late}
.scope n o p q (a b c d) {echo $a $b $c $d}
.scope n o p q (a b c d) {echo a b c d}
.scope n o (a [b] c [d]) {echo $a $c }
.scope n o ([a] b [c] d) {echo $b $d; .var_exists a}
.scope n o ([a] b [c] d) {echo $b; .var_exists c}
.scope n ([a1 a2 a3 a4] b [c] [d1 d2] [e ...]) {echo $b}
.scope n ([a1 a2 a3 a4] b [c] [d1 d2] [e ...]) {echo $b; .var_exists a4 d2 e c}
.scope n ([a1 a2 a3 a4] b [c] [d1 d2] [e ...]) {echo $b; .var_exists a3 d1}
.scope n ([a1 a2 a3 a4] b [c] [d1 d2] [e ...]) {echo ${.list_locals} $b}
.scope n o p q r s ([a1 a2 a3 a4] b [c] [d1 d2] [e ...]) {echo ${.list_locals} $b}
.scope n o p q r s t u ([a1 a2 a3 a4] b [c] [d1 d2]) {echo $a1 $d2}
.scope n o p q r s t u ([a1 a2 a3 a4] b [c] [d1 d2]) {
  echo $a1 $a2 $a3 $a4 $c $d1 $d2}
.scope ([-a a1 a2 a3] [-b] [-c] [-d d1]) {}
.scope ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {}
.scope ([-a a1 a2 a3] [-b] [-c] [-d d1]) {.var_exists -a -b -c -d}
.scope ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-?; .var_exists -a -b -c -d}
.scope ([-a a1 a2 a3] [-b] [-c] [-d d1]) {.list_locals}
.scope -b -d darg ([-a a1 a2 a3] [-b] [-c] [-d d1]) {.list_locals; echo ()}
.scope ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {echo ${.list_locals} $-?}
.scope -b -d darg -v ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {echo ${.list_locals} $-?}
.scope ([-a a1 a2 a3] [-d d1 d2]) {.var_exists a2 d2}
.scope ([-?] [-a a1 a2 a3] [-d d1 d2]) {.var_exists -? a3 d1}
.scope ([-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-*}
.scope ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-*}
.scope n ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1] [e] f) {.local loc lval; echo $-*}
.scope -c ([-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-c}
.scope -b -c ([-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-c; .var_exists -a -b -d}
.scope -b -c ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $-c; .var_exists -a -b -d -?}
.scope -b -c ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $-c $-?; .var_exists -a -b -d}
.scope -b -c ([-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-b; .var_exists a2 d1}
.scope -a n o p -b -c -d q ([-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $-c; .var_exists -a -b -d}
.scope -a n o p -b -c -d q ([-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $-b; .var_exists a2 d1}
.scope -a n o p -b -c -d q ([-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $-a $-b $-c $-d}
.scope -a n o p -b -c -d q ([-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $a1 $a2 $a3 $d1; .var_exists -b -c}
.scope -a n o p -b -c -d q ([-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $-*}
.scope -v -a n o p -b -c -d q ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {
  echo $a1 $a2 $a3 $d1; .var_exists -b -c}
.scope -v -a n o p -b -c -d q ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-?}
.scope -v -a n o p -b -c -d q ([-?] [-a a1 a2 a3] [-b] [-c] [-d d1]) {echo $-*}
.scope -a n o p ([-a a1 a2 a3]) {echo $a1 $a3}

# exiting
# rwsh.shutdown .exit
.exit excess_argument
.exit {excess argfunction}
.exit
.echo 1 2 3
