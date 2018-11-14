.binary /bin/cat
/bin/cat /tmp/lcl
.function_all_flags ## [args ...] {.nop $args}
#
.nop beware the empty comment

## argm
# .source
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
.mapped_argfunction {	   .echo ignore leading tab in argfunction}
.whence_function .mapped_argfunction {.nop}
.whence_function .argfunction {
  multiple line argfunction }
.nop .argfunction .mismatched_brace } &&&is$not$all
.source
.source /etc/hosts {excess argfunc}
.source test_files/*fu*bar*
.source /etc/hosts
.source test_files/unclosed_brace_newline.rwsh
.source test_files/unclosed_brace.rwsh
.source test_files/unclosed_parenthesis_newline.rwsh
.source test_files/unclosed_parenthesis.rwsh
.source test_files/multiple_errors.rwsh
.nop multiple statements \; on a line
.whence_function .argfunction {.multiple_argfunctions} {}
.whence_function .argfunction {.argfunction with text args}
.whence_function .argfunction {.argfunction {with a function arg}}
.whence_function x {.escaped_argfunction me}
.whence_function .argfunction {{{{{{{{{{{}}}}}}}}}}}
.whence_function .argfunction {
  .function_all_flags  x  { ${ .which_path  echo  $PATH }  something } }

## ability of functions to perform custom control flow
# rwshrc-basic
.function e {.echo $*}
.function om {.argfunction}
.function_all_flags sa [args ...] {
  .scope $args$ ([args ...]) {.argfunction}}
.function_all_flags se {.scope () {.argfunction}}
.whence_function e
.whence_function om
.whence_function sa
.whence_function se
whence e
whence ee
whence () {}
e text that does not have a prompt appended
se {e again}
if_only .test_is_number false {e not printed}
if_only .test_is_number 0 {e printed without error}
.function_all_flags for -- [items ...] {
  .if .var_exists items {.for $items$ {.argfunction}}
  .else {.nop}}
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
e a )mismatched &&parenthesis
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

# .init, rwshrc's autofunction, .binary
whence .init
whence .autofunction
.binary {e excess argfunction}
.binary /bin/rzwsh
.binary /bin/cat
.global PATH /usr/bin
/bin/true
true
.binary /bin/true
true
.rm_executable /bin/true
.set PATH /bin:/usr/bin:.
fals
/bin/true add the binary by itself
true add the function using existing binary
.whence_function /bin/false
false add the function and the binary at once
.whence_function true
/bin/echo this will add /bin/echo to the hash table
/bin/echo this will use the existing /bin/echo reference
echo this function is already defined
.whence_function echo
.autofunction ./rwsh
.whence_function ./rwsh
./rwsh -c (echo in a subshell)
./rwsh -c (false)
./rwsh -c (.get_pid)
./rwsh /non-existent/file/to/test/failure <test_files/pause_hello.rwsh
./rwsh test_files/pause_hello.rwsh <test_files/pause_hello.rwsh
./rwsh test_files/signal_triggered.rwsh <test_files/pause_hello.rwsh
.autofunction test_files/../rwsh
whence test_files/../rwsh

.function_all_flags .autofunction -- cmd [args ...] {
  # $args$ redefining .autofunction tested by subsequent usage
  .local full_path ${.which_path $cmd $PATH}
  if_only_not .test_executable_exists $full_path {.binary $full_path}
  if_only_not .test_string_equal $cmd $full_path {
    .function_all_flags $cmd -- [args ...] {
      &&full_path $args$
      echo this &&cmd function has been brought to you by .autofunction}}
  echo added $cmd to executable map in test suite}
.binary /bin/rm

# selection_read read_dir()
e @/etc
e @test_files/ixx
e @test_files/i*xx
e @test_files/i**xx
e @test_files/ii**xx
e @test_files/ix*xx
e @test_files/ix*xx*
e @test_files/ix*xxx
e @test_files/ix*xxx*
e @test_files/*iixxx
e @test_files/ix*x*x
e @test_files/ix*xx*x
e @test_files/ix*x*xx
.scope () {e @test_files/i*xx/f*}
.scope () {e @/*selection_not_found*}
e @/ur/bin/pwd
e @test_main.cc
se {se {.for @e*c {e $1 $nl}} >test_files/tmp}
.global LC_ALL C
.nop $LC_ALL
sort test_files/tmp
whence sort
e @test_files/*xx
e @test_files/*x*x*x*x
e @test_files/*xyxy
e @/bin
se {se {.for @/usr/*bin {e $1 $nl}} >test_files/tmp}
sort test_files/tmp
se {se {.for @/etc/rwsh* {e $1 $nl}} >test_files/tmp}
sort test_files/tmp
e @/etc/rw*ic
e @/etc/rwsh*a*
.set FIGNORE rwshrc* rwshrc-bas rwshrc-default
e @/etc/rwshrc-*
e @/etc/rwshrc*
e @/etc/rwshr*
.set FIGNORE *.cc
e @*cc
e @*h.cc
.set FIGNORE *de*
e @/etc/rwshrc-defa*
e @/etc/rwshrc-d*
e @/etc/rwsh*a*
e @test_files/*i*x*y*y*x*
e @/etc/rw*-basi*si*
e @test_main.sh
se {se {.for @test_files/*x* {e $1 $nl}} >test_files/tmp}
sort test_files/tmp
.scope r*h.cc sel*.h (A ...) {e @$A}

# Arg_spec::REFERENCE, interpret(), evaluate_expansion(), evaluate_var()
.nop $A
.set A /bin
.global B ( (zero   zero) ((one one  ) one   ) two three)
.global C ((external) () ( ) internal(parenthesis))
.global broken (extra_close\) \(extra_open)
e $A $0 @$A
e A $1 1 $$3 $$$3
e A $1a
e A 1 2 3 4 5 6 7 $$$$$$$$$8
sa $UNDECLARED $ALSO_UNDECLARED {}
sa {e $UNDECLARED $ALSO_UNDECLARED}
e &UNDECLARED &ALSO_UNDECLARED
.scope [undefined] {echo $undefined and also unchecked}
.scope [udef] {echo but $udef$ can vanish}
.scope ([-x foo bar]) {echo along with specific $-x$ flags}
.scope ([-x foo bar]) {echo note even one $bar$ prevents an error}
.scope [also_undefined] {echo even &&also_undefined$ in a soon}
.scope undeclared ([leading_ud] ref) {echo references $$ref$ can throw}
.scope leading_ud ([leading_ud] ref) {echo good $$ref$ ones are checked}
sa $B$$1x {e $# $args$}
sa $B$$1$ {e $# $args$}
sa $B {e $# $args$}
sa $B$ {e $# $args$}
sa $B$$ {e $# $args$}
sa $B$$$$ {e $# $args$}
se {sa $B$$$$ {e $# $args$}}
sa $B$10 {e $# $args$}
sa $B$1 {e $# $args$}
sa $B$$1 {e $# $args$}
om $broken {e $# $*}
sa $broken$ {e $# $args$}
sa $broken$$ {e $# $args$}
sa $C {e $# $args$}
sa $C$ {e $# $args$}
sa $C$$ {e $# $args$}

# Arg_spec::SOON, apply()
e A &1 1 &$3 &$$3
e &&A
se {e &&&A}
se {@{} e &&&without$A $.{mismatched} {.argfunction brace} &&&{thrown}B
}
e &{e &&A}
e &&{e &A}
e &A
.scope not_bin A {e &A &&A $A $nl; .scope otherwise A {e &A &&A &&&A $A}}
se {sa &B$10 {e $# $args$}}
se {sa &B$$$$ {e $# $args$}}
se {sa &B$1 {e $# $args$}}
se {sa &B$$1 {e $# $args$}}

# Arg_spec::SUBSTITUTION and Arg_spec::SOON_SUBSTITUTION, apply(), interpret(),
# evaluate_substitution()
e ${e $A}
whence .argfunction {e ${e $A}}
.scope not_bin A {
   e &{.echo $A} &&{.echo $A} $A $nl
   .scope otherwise A {
      e &{.echo $A} &&{.echo $A} &&&{.echo $A} ${.echo $A} $A}}
.scope not_bin A {e &{.echo &A $A} &&{.echo &A &&A} ${.echo &A $A}}
sa &{.echo $A} {e $args &1}
sa &{.return 1} {}
sa ${.return 1} {}
se {e &{.return 1}}
se {e &&{.return 1}; e after}
fn .failed_substitution [args ...] {.nop $args$; e $Z}
.throw sa {echo even from $args$ 7 is a number}
.fallback_handler sa {echo even from $args$ 7 is a number}
.throw .failed_substitution sa {echo even from $args$ 7 is a number}
se {e &&{.return 1}; e after}
e x{e bad argfunction style}
e x&&&{e x}
e $+{e x}
e &+{e x}
e &&${e x}
.return &{.return 0}
.return ${e 0 $nl}
.return &{.echo 0}
e nevermore &{/bin/echo quoth the raven} 
sa ${e $B}$@1 ${e $B}$1 ${e $B}XYZ {e $# $args$}
sa ${e $B} {e $# $args$}
sa &{e $B} {e $# $args$}
sa ${e $B}$ {e $# $args$}
sa &{e $B}$ {e $# $args$}
e $# &{e $B}$
se {e $# &{e $B}$}
sa &{e $B}$$ {e $# $args$}
sa ${e $B}$$ {e $# $args$}
se {sa ${e $B}$1 {e $# $args$}}
se {sa &{e $B}$1 {e $# $args$}}
se {sa ${e $B}$$$1 {e $# $args$}}
se {sa &{e $B}$$$1 {e $# $args$}}
.unset A
.unset B
sa &{e ((zero zero) (one one) two three)}$10 {e $# $args$}
sa ${e (zero zero) \)one one two three}$1 {e $# $args$}
sa &{e (zero zero) \(one one two three}$1 {e $# $args$}
om ${e (zero zero) \)one one two three} {e $# $*}
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
/bin/cat <non_existent_file
.for_each_line <non_existent_file {e line of $# \( $* \)}
se {sa {e hi >one >two} {cat <three <four}
}
.if .test_file_exists outfile {.echo failed to remove outfile}
.else {.echo outfile properly removed}
se {e hi >outfile}
/bin/cat outfile
.if .test_is_number 0 {>outfile /bin/echo there}
.else {.nop}
/bin/cat outfile
se {se >outfile {e line 1 $nl; e line 2 longer $nl; .echo $nl; e ending}}
/bin/cat <outfile
.for_each_line x {}
.for_each_line <outfile
.for_each_line <outfile <another A{}
.for_each_line <outfile {e line of $# \( $* \) $nl}
/bin/rm outfile

# soon level promotion
.global A 0
.global OLD_NESTING ${.get_max_nesting}
.set_max_nesting 46
fn x {.var_add A 1
     se {.var_add A 1
        se {.var_add A 1
           se {.var_add A 1
              se {.var_add A 1
                 se {.argfunction}}}}}}
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
.rm_executable x
.function_all_flags not soon {&&&enough}
.set_max_nesting $OLD_NESTING
.unset A
.unset OLD_NESTING

## builtin tests
# .argc
.argc {excess argfunc}
.argc
.argc ()
.argc (1 a) (2 b (c d)) (3 ((e () f)))

# .cd (.which_path ../)
.cd
.cd /bin {excess argfunc}
.cd /bin /
.cd /bn
.cd /bin/rwsh
.fork .mapped_argfunction {
  .try_catch_recursive .binary_not_found {.which_path ../bin/rwsh /bin:.}
  .cd /bin
  .binary /bin/pwd
  echo directory is now ${/bin/pwd}$ in subshell
  .try_catch_recursive .binary_not_found {.which_path ../bin/rwsh /bin}
  .try_catch_recursive .binary_not_found {.which_path ../bin/rwsh /bin:.}
  }

# .combine
.combine
.combine something {excess argfunc}
.combine on () e \ two ( ) $#

# .echo .error
.error
.error something {excess argfunc}
.nop .error cannot test error with a diff until you can redirect it $nl
.echo
.echo something {excess argfunc}
.echo these are fixed strings

# .exec .fork Binary Old_argv_t .binary_not_found
.fork
.fork e text
.fork .return 127
.fork sa 126 {sa $args$ {echo about to return $args$; .return $args$}}
.exec
.exec something {excess argfunc}
.exec /bin/ech does not exist
.exec /etc/rwshrc-default config files are not interpreters
.exec /bin cannot exec a directory
.exec /bin/rwsh/insanity errno is ENOTDIR
.fork se {.exec /bin/rwsh/insanity errno is ENOTDIR}
.fork se {.exec /bin/echo something; /bin/echo else}
.scope () {.fork se {.exec /bn/echo 1 2 3}}
.fork se {/bn/echo 1 2 3}
.last_exception /bn/echo
.exec /bn/echo 1 2 3
/bn/echo 1 2 3
.last_exception /bn/echo
.fork se {/bn/echo 1 2 3}
.fork .exec /bn/echo 1 2 3
/bn/echo 1 2 3
.exec /bn/echo 1 2 3
/bin/echo 1 2 3

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

# .function_all_flags .rm_executable
.function_all_flags
.function_all_flags missing argfunction
.rm_executable {excess argfunction}
.function_all_flags .exit {.nop}
.function_all_flags .escaped_argfunction {.nop}
.rm_executable a
.function_all_flags a {.nop}
.whence_function a
a 1 2 3
.rm_executable a
.whence_function a
.global A \
.echo () A () A () $#
.function_all_flags a arg {e 9 $A $arg @/usr}
.whence_function a
a \
a 1
a 1 2
.function_all_flags a [args ...] {.nop $args$; e $*5 $* $*0 $nl}
a
a 1
a 1 2
fn g name {.function_all_flags $name name {
  .function_all_flags $name {.argfunction}}}
g a {e 3 2 1 $nl}
whence a
a b
b
# a function redefining itself doesn't seg fault
fn g {e hi $nl; fn g {e there $nl}; fn h {e nothing here}; g}
g
.function_all_flags
.function_all_flags .exit {e cannot redefine a builtin as a function}
.function_all_flags .a {can define a function for non-existant builtin}
.function_all_flags .argfunction {e cannot define .argfunction}
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
.rm_executable nonsense
whence test_var_greater
.scope 5 n {test_var_greater n}
.scope 5 n {test_var_greater n 3 12}
.scope 5 n {test_var_greater n 3}
whence ntimes
ntimes -- 3 {e $n remaining $nl}
ntimes 2 {ntimes 3 {e &&n and $n remaining $nl}}
.function_all_flags a [-x] [-] [--long-opt y second {
  e mismatched bracket (i.e. missing close brakcet)}
.function_all_flags a [-?] [--] {.list_locals}
.function_all_flags a [-x] [--] foo {.list_locals}
.function_all_flags a [-?] -- foo {.list_locals}
.function_all_flags a [-x] -- {.list_locals}
.function_all_flags a [--] {.list_locals}
whence a
a
a --
a foo
.function_all_flags a [-x] [--long-opt y] second {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a
a single
a -x single
a --long-opt arg single
a --long-opt single
a --long-opt first -x --long-opt second single
.function_all_flags a [-q option1 option2] [-x o1 o2 o3 o4] required {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
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
whence a
a
a single
a one two
a one two three
a one two three four five
a one two three four five six seven eight nine
.function_all_flags a -y [second] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a
a 1
a 1 2
a 1 2 3
.function_all_flags a [-x] -y [--long-opt] second [--] [-] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
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
whence a
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
whence a
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
whence a
a
a -to -- -first
a -to -first
.function_all_flags a [-?] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }
  e nothing_required}
whence a
a
.function_all_flags a ... y {}
.function_all_flags a [-x] [--file file] ... y {}
.function_all_flags a [...] x {}
.function_all_flags a [... y] x {}
.function_all_flags a x ... [y z] {}
.function_all_flags a x [... y] z ... {}
.function_all_flags a [y z] ... x {}
.function_all_flags a [y] ... x {}
whence a
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
whence a
a -c -a -b first second third
a -c first -a second -b third
.function_all_flags a -* x ... y {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a -c -a -b first second third
a -c first -a second -b third
.function_all_flags a [-?] x ... y {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a 
a first
a first (se cond)
a first (se cond) third
a first (se cond) third fourth (fi fth)
.function_all_flags a [-?] x [...] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a
a first second third fourth fifth
.function_all_flags a [-?] x [--] [y ...] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a
a first
a first second
a first second third
a first second third fourth fifth
.function_all_flags a [-?] a [b ... c] d {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a first
a first second
a first second third
a first second third fourth
a first second third fourth fifth
a first second third fourth fifth sixth
.function_all_flags a [-x ...] b c {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a first second
a -x first
a -x first second
a -x first second third
a -x first second third fourth
.function_all_flags a [-x b ...] c {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
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
whence a
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
whence a
a
a (fi rst)
a (fi rst) second
a first (sec ond) third
a (fi rst) (sec ond) third fourth
a () (sec ond) third fourth
a (fi rst) (sec ond) (thi rd) (fou rth) (fi fth)
.function_all_flags a [-?] [x] [... y] {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
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
whence a
a
a (fi rst)
a (fi rst) second
a (fi rst) (sec ond) (thi rd)
a (fi rst) (sec ond) third (fou rth)
a (fi rst) (sec ond) (thi rd) (fou rth) (fi fth)
.function_all_flags a -- [-x] y z {}
.function_all_flags a -- [x y] z {
  for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
whence a
a
a --file
a first second third
a -x second -

# .get_max_collectible_exceptions .set_max_collectible_exceptions
.get_max_collectible_exceptions excess
.set_max_collectible_exceptions
.get_max_collectible_exceptions {excess}
.set_max_collectible_exceptions 1 {excess}
.set_max_collectible_exceptions NaN
.get_max_collectible_exceptions
.set_max_collectible_exceptions 0
.get_max_collectible_exceptions
.set_max_collectible_exceptions 7
.collect_errors_except
.collect_errors_only
se {.collect_errors_except .nop {
     se {echo before exception
        .throw .not_a_number 7
        echo after exception}
     echo between exceptions
     .scope () {.throw .function_not_found foo}
     echo inside collect}
   echo outside collect}
se {.collect_errors_except echo {
     .throw .function_not_found foo
     echo between exceptions
     .throw echo 7
     echo inside collect}
   echo outside collect}
# .collect_errors_except .echo {${.throw .echo exception thrown directly}}
se {.collect_errors_only .function_not_found {
     .throw .function_not_found foo
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
.get_max_extra_exceptions
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
.set_max_collectible_exceptions 7
.get_max_collectible_exceptions

# .getpid .getppid
.getpid excess
.getpid {excess argfunc}
.getppid excess
.getppid {excess argfunc}
.fork se {/bin/kill ${.getppid}}
.fork se {/bin/kill ${.getpid}}
se {.fork se {/bin/kill ${.getppid}
            echo after the signal in subshell}
   echo after the signal in parent}
se {.fork se {/bin/kill ${.getpid}
            echo after the signal in subshell}
   echo after the signal in parent}

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
.function_all_flags a {if_only .var_exists x {e in a x \( $x \) $nl}
             if_only .var_exists y {e in a y \( $y \) $nl}
             .local x (first level not global)
             .local y (level one not global)
             b
             if_only .var_exists x {e out a x \( $x \) $nl}
             if_only .var_exists y {e out a y \( $y \) $nl}}
.function_all_flags b {if_only .var_exists x {e in b x \( $x \) $nl}
             if_only .var_exists y {e in b y \( $y \) $nl}
             .local x (second level masks first)
             .set y   (level two overwrites one)
             c
             if_only .var_exists x {e out b x \( $x \) $nl}
             if_only .var_exists y {e out b y \( $y \) $nl}}
.function_all_flags c {if_only .var_exists x {e in c x \( $x \) $nl}
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

## if_core
# helper functions
.function_all_flags silent_throw [text ...] {
  .collect_errors_only .false echo {
    .throw .false first
    .throw echo exception $text$
    .throw .false second
    .return 0}}
.function_all_flags caught_silent_throw [text ...] {
  .try_catch_recursive echo {silent_throw $text$}}
silent_throw on its own
caught_silent_throw on its own
fn if_true {.if .test_not_empty ( ) {.argfunction}}
fn if_false {.if .test_not_empty ()  {.argfunction}}
fn else_if_true {.else_if .test_string_unequal q w {.argfunction}}
fn else_if_false {.else_if .test_string_unequal q q {.argfunction}}
fn else_if_not_true {.else_if_not .test_string_equal x x {.argfunction}}
fn else_if_not_false {.else_if_not .test_string_equal y z {.argfunction}}
fn obscured_else {.scope () {.scope () {.else {.argfunction}}}}
fn if_with_body args ... {.if $args$ {echo the afore-mentioned body}}
fn if_else_if_not_with_body args ... {
  .if $args$ {echo the first body}
  .else_if_not $args$ {echo the second body}}
fn double_if_with_body args ... {
  .if $args$ {echo the first body}
  .if $args$ {echo the second body}}
fn else_if_with_body args ... {.else_if $args$ {echo the afore-mentioned body}}
fn else_with_body [args ...] {.else {echo the afore-mentioned &&args$}}
fn conditional_true {
  .if .test_in . . {echo in_conditional_if}
  .else {echo in_conditional_else}
  .test_in . .}
fn conditional_false {
  .if .test_in . {echo in_conditional_if}
  .else {echo in_conditional_else}
  .test_in .}
fn conditional_echo first second {
  .if .test_in () {echo not printed if empty string in null set}
  .else_if_not .echo $first {echo not printed else if echo fails}
  .else {.echo () $second}}

# ! negation: if_core workout
.function_all_flags !x args ... {
  .if .throw .false anyway {}
  .else_if_not $args$ {.return 0}
  .else {.throw .false ${.echo ! $args}}}
.test_string_equal 42 420e-1
.test_number_equal 42 420e-1
!  .test_greater 50 230
!x .test_greater 50 230
!  .test_less 5 2.3e1
!x .test_less 5 2.3e1
!  !  .test_executable_exists !!!
!  !x .test_executable_exists !!!
!x !  .test_executable_exists !!!
!x !x .test_executable_exists !!!
!  !  .whence_function !!!
!x !x .whence_function !!!
!  !  .test_not_empty 0
!  !x .test_not_empty 0
!x !  .test_not_empty 0
!x !x .test_not_empty 0
!  !  !  .test_in 0 0
!  !  !  .test_in 0
!  !  !  !  .test_in 0
!  !  !x !  .test_in 0
!  !x !  !  .test_in 0
!  !x !x !  .test_in 0
!x !  !  !  .test_in 0
!x !  !x !  .test_in 0
!x !x !  !  .test_in 0
!x !x !x !  .test_in 0
!  !  !  !  .test_in 0 0
!  !  !x !  .test_in 0 0
!  !x !  !  .test_in 0 0
!  !x !x !  .test_in 0 0
!x !  !  !  .test_in 0 0
!x !  !x !  .test_in 0 0
!x !x !  !  .test_in 0 0
!x !x !x !  .test_in 0 0

# properly sequenced un-nested conditionals where everything succeeds
if_true {e if_true printing}
else_if_true {e else_if_true not printing when condition true}
else_if_not_false {e else_if_not_false not printing when condition true}
else_if_false {e else_if_false not printing when condition true}
else_if_not_true {e else_if_not_false not printing when condition true}
obscured_else {e obscured_else not printing when condition true}
if_false {e if_false not printing}
else_if_false {e else_if_false not printing when condition false}
else_if_not_true {e else_if_not_true not printing when condition false}
obscured_else {e obscured_else is printing when condition false}
if_false {e if_false not printing}
else_if_true {e else_if_true printing when condition false}
obscured_else {e obscured_else not printing when condition true}
if_false {e if_false not printing}
else_if_not_false {e else_if_not_false printing when condition false}
obscured_else {e obscured_else not printing when condition true}
if_with_body .test_not_empty a
else_if_with_body .test_not_empty a
else_with_body
if_with_body .test_not_empty ()
else_if_with_body .test_not_empty ()
else_with_body text expanded
if_with_body .test_not_empty ()
else_if_with_body .test_not_empty a
else_with_body text

# nested conditionals
.if conditional_true {e conditional_true printing}
.else {e else not printing when condition true}
.if conditional_false {e conditional_false not printing}
.else {e else is printing when condition false}
.if ${conditional_echo .test_is_number 71}$ {e conditional_echo printing}
.else {e else not printing when condition true}
.if ${conditional_echo .test_is_number pi}$ {e conditional_echo not printing}
.else {e else is printing when condition false}
if_true {echo ${conditional_echo print this}}
.else {e else not printing when condition true}
if_false {echo ${conditional_echo .test_not_empty ()}}
.else {e else is printing when condition false}
.if conditional_true {echo ${conditional_echo print this}}
.else {echo ${conditional_echo not printing}}
.if conditional_false {echo ${conditional_echo not printing}}
.else {echo ${conditional_echo print this}}

# improperly sequenced conditionals and conditional block exception
.else {echo top level else without if}
.collect_errors_except .nop {
  .if else_with_body {echo else without if in condition}
  else_if_true {echo else_if ignored after exception}
  else_if_false {echo else_if ignored after exception}
  else_if_not_true {echo else_if_not ignored after exception}
  else_if_not_false {echo else_if_not ignored after exception}
  obscured_else {echo else skipped when properly closing block}
  echo end collect errors}
.collect_errors_except .nop {
  if_true {.else {echo else without if in body}}
  echo else is skipped without issue (expecting exception avoided it)}
.collect_errors_except .nop {
  if_false {echo set up}
  else_if_true {else_if_true {else_if without if}}
  else_with_body}
.collect_errors_except .nop {
  double_if_with_body .test_in if following another top-level if
  if_true {echo if_true printing though it is the third if}
  else_with_body}
.collect_errors_except .nop {
  if_with_body double_if_with_body .test_in double if in condition
  obscured_else {echo else skipped when properly closing block}}
.collect_errors_except .nop {
  if_false {}
  .else {double_if_with_body .test_in double if in body}
  .else {echo double else is an exception even when first throws}
  if_true {echo if_true printing immediately after an else that threw}
  else_with_body}
.collect_errors_except .nop {
  if_true {.else {echo else without if}}
  .else {echo ignored because it is at the level of the if
         echo not the else that it immediately follows (if-else ambiguity)}
  if_false {echo if ignored because the condition is false}
  else {echo else run after the properly closed if-else}}
.collect_errors_except .nop {
  if_false {}
  obscured_else {if_false {echo bad_if_nest in condition body}}
  else_if_true {echo exception for else_if after else that threw}
  else_if_not_false {
    echo else_if_not ignored after else_if that threw without else following}
  .if if_with_body if_with_body .test_in innermost if {echo is not matched}}
.if if_with_body conditional_true {echo middle if is not matched}
.if if_else_if_not_with_body .test_in else if without else {
  echo body is not run with exception in condition}
.collect_errors_except .nop {
  if_false {}
  obscured_else {if_else_if_not_with_body .test_in else_if without else in body}}
.collect_errors_except .nop {
  if_false {}
  .else {if_true {if_true {echo unmatched if doubled in body}}}}
.collect_errors_except .nop {
  if_false {}
  .else {.if if_with_body .test_in unmatched {echo if in condition of body}}}
.collect_errors_except .nop {
  .try_catch_recursive echo {
    .if silent_throw in if {echo definitely should not print}
    .else {echo skipped because non-.false exception thrown earlier}}}
.collect_errors_except .nop {
  .try_catch_recursive echo {
    .if caught_silent_throw in if {echo still should not print}
    .else {echo not skipped because only .false were thrown earlier}}}

# exception thrown in argfunction
.if ${conditional_echo unfindable anywhere}$ {e also a failed substitution}
.else {e else skipped when condition threw an exception}
if_true {echo &&{unfindable anywhere}}
.else {e else not printing when condition true and if body threw}
if_true {echo &{unfindable anywhere}}
.else {e else throws an exception because we never got into the if}

# .if .else_if .else_if_not .else
.if
.else_if .test_is_number 0 {e do not run after an exception}
.else_if_not .test_is_number false {e do not run after an exception}
.else_if
.else_if_not
.else {e do not run after an exception}
.if missing argfunction
.else_if missing argfunction
.else_if_not missing argfunction
.else
.if .test_is_number 0 {}
.else {e first else for if}
.else {e second else for if}
.if .test_is_number 0 {e if true}
.else {e else true; .return 3}
.if .test_is_number false {e if false}
.else {e else false; .return 5}
.else {}
.else_if .test_is_number 0 {e not this one}
.else {}
.else_if_not .test_is_number false {e nor this}
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
se {.is_default_input <dummy_file}
se {.is_default_input}
.is_default_output 1
.is_default_output {excess argfunc}
e ${.is_default_output; .echo $?}
se {.is_default_output >dummy_file}
se {.is_default_output}
.is_default_error 1
.is_default_error {excess argfunc}
se {.is_default_error}

# .list_executables
.list_executables excess
.list_executables {excess argfunc}
.for ${.list_executables}$ {.combine $1 $nl}

# .ls
.ls
.ls /bin {excess argfunc}
.ls /bin/usr/
.ls /bi  /us /nr
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

# .scope
.scope {e $foo}
.scope () {.scope foo}
.scope a (y y) {e illegal duplicate required parameter}
.scope a ([-x] [-x]) {e illegal duplicate flag parameter}
.scope a ([x x]) {e illegal duplicate optional parameter}
.scope a ([-x arg bar] [-y arg]) {e illegal duplicate flag argument}
.scope a (-x [-x]) {e evil duplication between flags positional}
.scope -- -- {e -- as a prototype forbids even -- as arguments}
.scope -- () {e but ${.list_locals} is acceptable for empty prototype}
.scope () {e no arguments are also good for empty prototype}
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
whence a
.function_all_flags pt -- args ... {
  .scope $args$ ([-x] [--long-opt y] second) {
    for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
  .combine $nl
  .scope $args$ ( [-?] [--long-opt y] second) {
    for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}}
whence pt
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
whence a
.function_all_flags pts -- [args ...] {
  .if var_exists args {
    .scope $args$ ([-first] [-?]) {
      .for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
      e nothing_required}
  else {.scope ([-first] [-?]) {
      .for ${.list_locals}$ {.combine $1 \( $$1 \) \ }}
      e nothing_required}}
whence pts
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
.scope [undefined] {.collect_errors_except .nop {
  .set undeclared is an error
  echo $undeclared}}
.scope [undefined] {.collect_errors_except .nop {
  .set undefined and also unchecked
  echo $undefined}}
.scope but_ignored [defined] {.collect_errors_except .nop {
  .set defined but also unchecked
  echo $defined}}
.scope unchanged [defined] {.collect_errors_except .nop {
  .set defined unchanged
  echo variable $defined by a set does not throw}}
.scope [undefined] {.collect_errors_except .nop {
  if_only_not .var_exists undefined {.set undefined is checked}
  echo $undefined}}
.set B x {excess argfunc}
.set A x
e $A

# .try_catch_recursive .get_max_extra_exceptions .set_max_extra_exceptions
.try_catch_recursive .function_not_found 
.try_catch_recursive {.return A}
fn e_after {se {.argfunction}; echo after}
e_after {.try_catch_recursive .not_a_number .function_not_found {
  .return A}}
e_after {
  .try_catch_recursive .function_not_found .binary_not_found {
    ..return A}}
e_after {.try_catch_recursive .not_a_number {.return A}}
e_after {.try_catch_recursive .not_a_number {.cho A}}
e_after {.try_catch_recursive .not_a_number .function_not_found {echo A}}
e_after {sa echo hi {.try_catch_recursive ${.internal_functions}$ {&&&args$}}}
.get_max_extra_exceptions excess
.set_max_extra_exceptions
.get_max_extra_exceptions {excess}
.set_max_extra_exceptions 1 {excess}
.set_max_extra_exceptions NaN
.set_max_extra_exceptions -1
.get_max_extra_exceptions
.set_max_extra_exceptions 0
.get_max_extra_exceptions
e_after {.try_catch_recursive .not_a_number {.try_catch_recursive .not_a_number {.return A}}}
e_after {.try_catch_recursive .not_a_number {.try_catch_recursive .not_a_number {.return A}}}
e_after {sa echo hi {.try_catch_recursive ${.internal_functions}$ {&&&args$}}}
.set_max_extra_exceptions 5
.get_max_extra_exceptions

# .stepwise
.function_all_flags wrapper args ... {a $args$ two; a $args$ three}
.function_all_flags a args ... {e $args$ one $nl; e $args$ two $nl
  e $args$ three $nl}
.function_all_flags d args ... {e $args$ $nl; .stepwise $args$ {d $*}}
.stepwise {e $* $nl}
.stepwise wrapper 1 2
.stepwise stepwise {e $* $nl}
.stepwise .stepwise {e $* $nl}
.stepwise wrapper 1 2 {e $* $nl}
.function_all_flags wrapper args ... {a $args$ one
  a $args$ two; a $args$ three}
wrapper 1 2
.stepwise wrapper 1 2 {d $*}
.stepwise wrapper 1 2 {e $* $nl}

# .test_file_exists
.test_file_exists
.test_file_exists dummy_file {excess argfunc}
.test_file_exists /b /in
.test_file_exists /b /bin /in

# .test_string_equal .test_string_unequal .test_not_empty .test_in
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
.test_in
.test_in x
.test_in x x
.test_in z y z
.test_in w x y z
.test_in y x y z

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

# .test_executable_exists .type .whence_function 
# Arg_script::str() but only an unknown fraction of the lines
# Arg_spec::str() (except trailing whitespace) only through SOON case
.test_executable_exists
.test_executable_exists j
.test_executable_exists #
.test_executable_exists .nop
.test_executable_exists /bin/cat
.test_executable_exists .mapped_argfunction
.test_executable_exists .mapped_argfunction {.argfunction}
.type
.type j
.type #
.type .nop
.type /bin/cat
.type .mapped_argfunction
.type .mapped_argfunction {.argfunction}
type .nop .mapped_argfunction j # {.argfunction}
type -t .nop .mapped_argfunction j # {.argfunction}
.whence_function
.whence_function j
.whence_function #
.whence_function .nop
.whence_function /bin/cat
.whence_function .mapped_argfunction
.whence_function .mapped_argfunction {.argfunction}
whence .mapped_argfunction {.nop 1 () \ \\ \$ \@ \) \(}
whence .mapped_argfunction {.nop 1 () \  \\ \$ \@ \) \(}
whence .mapped_argfunction {@a @$a @$1 @$* @$*2}
whence .mapped_argfunction {>dummy_file}
# new tests here
.whence_function .mapped_argfunction {{&&{&&x &&{e}}$$$ ${&&x ${e}}$$$ {&&&x &&&{e} {&&&&x &&&&{e}}}}}
whence .mapped_argfunction {$A $$A $0 $$$1 $# $* $*2 $A$$$ $A$10 $$*$ $$$*12$}
.whence_function .mapped_argfunction {&&A &&0 &&* &&*3 &&$A$$$ &&$A$10 &&*$ &&*6$ {&&&A$ &&&A$10}}
.function_all_flags wm [args ...] {
   .whence_function .mapped_argfunction {.argfunction}
   .echo $nl
   .scope $args$ (a1 a2 a3) {.argfunction}}
wm (aa ab ac) bb cc {
  e x &&a1 &2 $a3 y &&a1$1 z &&a1$2; .nop $a2; sa &&a1$ {e () w $args$ $#}}
.scope (aa ab ac) bb cc (args more ...) {.scope $args $more$ (args more ...) {
  sa &1 {echo $args$ $#}
  sa &&args {echo $args$ $#}
  sa &&&args {echo $args$ $#}
  sa $args {echo $args$ $#}
  sa $args$ {echo $args$ $#}
  sa $more$ {echo $args$ $#}}}
.function_all_flags wm [args ...] {
   whence .mapped_argfunction {.argfunction}
   .echo $nl
   .nop $args
   se {.argfunction}}
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
whence .mapped_argfunction {.nop 1}
whence .mapped_argfunction {@a @$a @$1 @$* @$*2}
whence .mapped_argfunction {>dummy_file}

# .waiting_for_shell .waiting_for_user .waiting_for_binary
# actual use of these builtins is in test_inconsistent.sh
.waiting_for_shell j
.waiting_for_shell {excess argfunc}
# .waiting_for_shell
.waiting_for_user j
.waiting_for_user {excess argfunc}
# .waiting_for_user
.waiting_for_binary j
.waiting_for_binary {excess argfunc}
# .waiting_for_binary

# .last_exception
.last_exception .last_exception
.last_exception .nop
.last_exception
.last_exception .try_catch_recursive {excess argfunc}
.last_exception .last_exception
.last_exception function_that_does_not_exist
.last_exception .try_catch_recursive
.last_exception .last_execution_time
.last_exception /bin/which
.last_exception /bin/cat
.last_exception /bin/echo
.last_exception e
.last_exception test_var_greater

# .usleep .execution_count .last_execution_time
# .total_execution_time
.usleep_overhead excess
.usleep_overhead {excess argfunc}
.usleep_overhead 
.usleep
.usleep 800 {excess argfunc}
.usleep -6
.usleep 5i
.usleep 800
# .usleep_overhead 
.execution_count
.execution_count j
.execution_count .mapped_argfunction {echo not tracked}
.execution_count .usleep
.last_execution_time
.last_execution_time j
.last_execution_time .mapped_argfunction {echo not tracked}
# .last_execution_time .usleep
.total_execution_time
.total_execution_time .mapped_argfunction {echo not tracked}
.total_execution_time j
# .total_execution_time .usleep

# .which_path
.which_path cat
.which_path cat /bin {excess argfunc}
.which_path cat \
.which_path does<not>exist /bin:/usr/bin
.which_path cat :/bin:/usr/bin
.which_path cat /usr/bin/:/bin/:
.which_path /bin/cat /usr/bin/:/bin/:
.which_path rwsh /usr/bin:/bin
.which_path rwsh .:/usr/bin:/bin
.which_path rwsh /usr/bin:/bin:.
.which_path ./rwsh /usr/bin:/bin
.which_path ./rwsh /bin:.
.which_path ../bin/rwsh /usr/bin:/bin
.which_path rwsh :/usr/bin::/usr/bin:
.which_path rwsh /usr/bin:.

# .while
.function_all_flags tf N {.test_string_unequal $A $N}
.set A 0
.while {e ARGS}
.while tf 4 
.while tf 4 {e printed; .set A 4}
.while tf 4 {e skipped}
.while .return 0 {.throw .echo exception within while}
.while silent_throw within condition {e body skipped}
.while .throw .false because {e body skipped}
.set A 0
.while tf 4 {e in .while argfunction $A $nl; .var_add A 1}
.set A 0
.while tf 4 {.if .test_number_equal 0 $A {.set A 1}
           .else {.function_all_flags tf N {.throw .false $N}}
           e in overwriting argfunction $nl}

# .var_add
.var_add
.var_add A 1 2
.var_add A 1 {excess argfunc}
.var_add B 1
.scope [undefined] {.var_add undefined 1}
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
.scope [undefined] {.var_divide undefined 1}
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
.scope [undefined] {.var_subtract undefined 1}
.set A A
.var_subtract A 2 
.set A 3000000000
.var_subtract A 2 
.set A 1e308
.var_subtract A -1e308
e $A
.set A -1e308
.scope () {.var_subtract A 1e308}
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

# internal functions 
# .after_command .raw_command .prompt
# all of these are used as part of the test itself. If this changes, then the 
# following tests will fail.
whence .after_command
whence .prompt
whence .raw_command

# .before_command
fn .before_command args ... {.echo $0 $args$; .echo $nl}
.rm_executable .before_command

# .double_redirection
# se {e hi #>dummy_file >dummy_file}
# se {e hi >dummy_file}
# se {e &{e hi #>dummy_file}}

# .function_not_found
se {se {not_a_thing; e should not be printed}}
.rm_executable .function_not_found
.whence_function .function_not_found
whence not_a_thing
not_a_thing
.whence_function .function_not_found

# .help .internal_error
.help
.internal_error techincally this is untestable

# .mapped_argfunction .unescaped_argfunction .argfunction
# .escaped_argfunction
.mapped_argfunction 1 2 3 {e a $* a}
.mapped_argfunction
fn g {.whence_function .argfunction {.unescaped_argfunction}
     .combine $nl
     .whence_function .argfunction {.argfunction}
     .combine $nl
     .whence_function .argfunction {.escaped_argfunction}}
g {}

# .excessive_nesting Base_executable::exception_handler
fn g {h}
fn h {g}
g
.stepwise g {e $* $nl; $*}
fn .excessive_nesting args ... {.nop $args; h}
g
fn .excessive_nesting args ... {.nop $args; e &&{.return 1}}
fn .failed_substitution args ... {.nop $args; e $Z}
g
.set_max_extra_exceptions 0
e_after {.try_catch_recursive .undeclared_variable .excessive_nesting .failed_substitution {g}}
.set_max_extra_exceptions 5
e_after {.try_catch_recursive .undeclared_variable .excessive_nesting .failed_substitution {g}}
e_after {.try_catch_recursive .undeclared_variable .failed_substitution {
  e ${.return 1}}}
fn .else_without_if args ... {.nop $args; e ${.return 1}}
e_after {.try_catch_recursive .undeclared_variable .else_without_if {
  .else {}}}
fn .failed_substitution args ... {.nop $args; e ${.return 2}}
e_after {.try_catch_recursive .undeclared_variable .failed_substitution {
  e ${.return 1}}}
fn .else_without_if args ... {.nop $args; e $Z}
e_after {.try_catch_recursive .undeclared_variable .else_without_if {
  .else {}}}
fn .failed_substitution args ... {.nop $args; .return Z}
e_after {.try_catch_recursive .not_a_number .failed_substitution {
  e ${.return 1}}}

# .throw
.throw
.throw .nop
.throw .not_a_number 7
se {if_only .test_is_number 0 {
  .throw sa {echo even from $args$ 7 is a number}}}
.throw .throw echo innocently
.throw .throw exception_bomb
se {.try_catch_recursive echo {
      echo first
      .throw echo failing successfully
      echo second}
   echo third}

# .run_logic
fn .run_logic flag cmd ... {
  .if .test_number_equal 0 $flag {.nop}
  .else_if $cmd$ {.nop}
  .else {.nop}}
0 e don't print nuthin'
1 e do print
1 .rm_executable .run_logic
1 e executable not found

# .vars
.vars

## recursive function testing
fn -n -- [args ...] {$args$; echo after $args$}
-n -n -n -n
-n -n -n echo -n
fn -n [-x] ignored -- [args ...] {
  echo before $args flags $-x$
  $args$
  .nop $ignored}
-n -x -x a -n -x a -n a -n -x a
-n -x -x a -n -x a -n a echo -n -x a
single .nop
single - - 40 10 - 7 2 1
single - 40 10
single - 40 - 10 2
single / - + 40 10 2 2
single - 40 - 10 - 7 2
.function_all_flags excessive-commentary arg {#! shebang
  $arg first comment
  echo not a $arg; .nop second; echo either}
excessive-commentary #
echo-comments excessive-commentary #

## environment testing i don't want to mess up everything else
# check for extraneous variables and that export_env doesn't bless
.scope M val (N necessary) {printenv; .nop $necessary}

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
printenv
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
# .shutdown .exit
.exit excess_argument
.exit {excess argfunction}
.exit
.echo 1 2 3
