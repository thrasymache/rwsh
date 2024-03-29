.binary /bin/cat
/bin/cat /tmp/lcl
.function ## [args ...] {.nop $args}
#
.nop beware the empty comment

## argm
# .source
.nop
     .nop
.nop 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
echo 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
.scope  ()    1                2       (a ...) {.echo $a$ $# $nl}
.scope \escaped internal\ space (a ...) {.echo $a$ $# $nl}
.scope now \ external\  (a ...) {.echo $a$ $# $nl}
.scope a \  space (a ...) {.echo $a$ $# $nl}
echo \$tokens \} \{ \; \\ \) \(
.echo a \
line continuation (or it was supposed to be)
 echo ignore leading space
	echo ignore leading tab
.mapped_argfunction {	   echo ignore leading tab in argfunction}
.whence_function .mapped_argfunction {.nop}
.whence_function .argfunction {
  multiple line argfunction }
.nop .argfunction .mismatched_brace } x{}}&&&is$not$all
.source
.source /etc/hosts {excess argfunc}
.source test_files/*fu*bar*
.source /etc/hosts
.source test_files/unclosed_brace.rwsh
.source test_files/unclosed_brace_without_newline.rwsh
.source test_files/unclosed_dot_brace.rwsh
.source test_files/unclosed_parenthesis.rwsh
.source test_files/unclosed_parenthesis_without_newline.rwsh
.source test_files/multiple_errors.rwsh
.source test_files/bad_substitution.rwsh
.nop multiple statements \; on a line
.whence_function .argfunction {.multiple_argfunctions} {}
.whence_function .argfunction {.argfunction with text args}
.whence_function .argfunction {.argfunction {with a function arg}}
.whence_function x {.escaped_argfunction me}
.whence_function .argfunction {{{{{{{{{{{}}}}}}}}}}}
.whence_function .argfunction {
  .function  x  { ${ .which_path  echo  $PATH }  something } }

## ability of functions to perform custom control flow
# rwshrc-basic
.function echoe -- text ... {.nop $text; .echo $*}
.function echow -- text ... {.echo $text$; .combine $nl}
.function sa [args ...] .{argfunction} {
  .scope $args$ ([args ...]) {.argfunction}}
.function se .{argfunction} {.scope () {.argfunction}}
.whence_function echoe
.whence_function sa
.whence_function se
whence echoe
whence echo
whence ee
whence () {}
echo text that does not have a prompt appended
se {echo again}
if_only .test_is_number false {echo not printed}
if_only .test_is_number 0 {echo printed without error}
foreok k {echo skipped without error}
foreok 1 2 3 k {echo loop $k}

## arg_script.cc and arg_spec.cc
# Arg_spec::FIXED, Arg_script::add_quote
echo 5 4 3 2 1
sa a (tight string created by parentheses $#) {echow $args$ $#}
sa a ( spaced string created by parentheses $# ) {echow $args$ $#}
.scope some escaped \) \(parentheses (a b c d) {echo $a $b $c $d $#}
sa some (nested (parentheses) $#) {echow $args$ $#}
sa some ((((((((((repeated))))) parentheses))))) {echow $args$ $#}
sa a (multi-line parenthesis
  enclosed string) {echow $args$ $#}
echo a )mismatched &&parenthesis
echo a (multi-line parenthesis
  mismatch))
echoe (internal \)parenthesis \\ escape ( \))); .combine $nl
.argc (internal \)parenthesis \\ escape ( \))); .combine $nl

# star_var (argm_star_var)
.scope 1 2 3 4 (a b c d) {echo $a $b $* $c $d \$*}
.scope 1 2 1 2 (a b c d) {echo $a $b $*3}

# star_soon
sa .nop 1 2 3 {
  .if $args$ {echo &&*}
  .else {}}
sa .nop 1 2 3 {
  .if $args$ {echo &&*0}
  .else {}}

# .init, rwshrc's autofunction, .binary
whence .init
whence .autofunction
whence .binary
.binary {echo excess argfunction}
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
.whence_function /usr/bin/env
env false add the function and the binary at once
.whence_function true
/bin/echo this will add /bin/echo to the hash table
/bin/echo this will use the existing /bin/echo reference
echo this function is already defined
.autofunction ./rwsh
.whence_function ./rwsh
./rwsh -c (echo in a subshell)
false
if_only_not false {echo false throwing a .false exception}
./rwsh -c (env false)
./rwsh -c (.get_pid)
./rwsh --init-file /non-existent/file/to/test/failure <test_files/pause_hello.rwsh
./rwsh --init-file test_files/pause_hello.rwsh <test_files/pause_hello.rwsh
./rwsh --init-file test_files/signal_triggered.rwsh <test_files/pause_hello.rwsh
./rwsh test_files/hello_argv.rwsh
./rwsh test_files/hello_argv.rwsh world
.autofunction test_files/../rwsh
whence test_files/../rwsh

.function .autofunction -- cmd [args ...] {
  # $args$ redefining .autofunction tested by subsequent usage
  .local full_path ${.which_path $cmd $PATH}
  if_only_not .test_executable_exists $full_path {.binary $full_path}
  if_only_not .test_string_equal $cmd $full_path {
    .function $cmd -- [args ...] {
      &&full_path $args$
      echo this &&cmd function has been brought to you by .autofunction}}
  echo added $cmd to executable map in test suite}
.binary /bin/rm

# selection_read read_dir()
echo @/etc
echo @test_files/ixx
echo @test_files/i*xx
echo @test_files/i**xx
echo @test_files/ii**xx
echo @test_files/ix*xx
echo @test_files/ix*xx*
echo @test_files/ix*xxx
echo @test_files/ix*xxx*
echo @test_files/*iixxx
echo @test_files/ix*x*x
echo @test_files/ix*xx*x
echo @test_files/ix*x*xx
.scope () {echo @test_files/i*xx/f*}
.scope () {echo @/*selection_not_found*}
echo @/ur/bin/pwd
echo @test_main.cc
forj @e*c {echo $j}
echo @test_files/*xx
echo @test_files/*x*x*x*x
echo @test_files/*xyxy
echo @/bin
forj @/usr/*bin {echo $j}
forj @/etc/rwsh* {echo $j}
echo @/etc/rw*ic
echo @/etc/rwsh*a*
.set FIGNORE rwshrc* rwshrc-bas rwshrc-default
echo @/etc/rwshrc-*
echo @/etc/rwshrc*
echo @/etc/rwshr*
.set FIGNORE *.cc
echo @*cc
echo @*h.cc
.set FIGNORE *de*
echo @/etc/rwshrc-defa*
echo @/etc/rwshrc-d*
echo @/etc/rwsh*a*
echo @test_files/*i*x*y*y*x*
echo @/etc/rw*-basi*si*
echo @test_main.sh
forj @test_files/*x* {echo $j}
.scope r*h.cc sel*.h (A ...) {echo @$A}

# Arg_spec::REFERENCE, interpret(), evaluate_expansion(), evaluate_var()
.nop $A
.set A /bin
.global B ( (zero   zero) ((one one  ) one   ) two three)
.global C ((external) () ( ) internal(parenthesis))
.global broken (extra_close\) \(extra_open)
echo $A $0 @$A
.scope A 1 (a b) {echo $a $1 $b $$2 $$$2}
echo A $1a
sa A 1 2 3 4 5 6 7 {echo $args$ $$$$$$$$$8}
sa $UNDECLARED $ALSO_UNDECLARED {}
sa {echo $UNDECLARED $ALSO_UNDECLARED}
echo &UNDECLARED &ALSO_UNDECLARED
.scope [undefined] {echo $undefined and also unchecked}
.scope [udef] {echo but $udef$ can vanish}
.scope ([-x foo bar]) {echo along with specific $-x$ flags}
.scope ([-x foo bar]) {echo note even one $bar$ prevents an error}
.scope [also_undefined] {echo even &&also_undefined$ in a soon}
.scope undeclared ([leading_ud] ref) {echo references $$ref$ can throw}
.scope leading_ud ([leading_ud] ref) {echo good $$ref$ ones are checked}
sa $B$$1x {echo $# $args$}
sa $B$$1$ {echo $# $args$}
sa $B {echow $# $args$}
sa $B$ {echow $# $args$}
sa $B$$ {echow $# $args$}
sa $B$$$$ {echo $# $args$}
se {sa $B$$$$ {echo $# $args$}}
sa $B$10 {echo $# $args$}
sa $B$1 {echow $# $args$}
sa $B$$1 {echo $# $args$}
.scope $broken arg {echoe $# $arg $nl}
sa $broken$ {echo $# $args$}
sa $broken$$ {echo $# $args$}
sa $C {echow $# $args$}
sa $C$ {echow $# $args$}
sa $C$$ {echo $# $args$}

# Arg_spec::SOON, apply()
.scope A 1 (a b) {echo $a &&1 $b &&$2 &&$$2}
echo &&A
se {e &&&A}
se {@{} e &&&without$A $.{mismatched} {.argfunction brace} &&&{thrown}B
}
echo &{e &&A}
echo &&{e &A}
echo &A
.scope not_bin A {echo &A &&A $A; .scope otherwise A {echo &A &&A &&&A $A}}
se {sa &B$10 {echoe $# $args$}}
se {sa &B$$$$ {echo $# $args$}}
se {sa &B$1 {echoe $# $args$} $nl}
se {sa &B$$1 {echo $# $args$}}

# Arg_spec::SUBSTITUTION and Arg_spec::SOON_SUBSTITUTION, apply(), interpret(),
# evaluate_substitution()
echo ${e $A}
whence .argfunction {e ${e $A}}
.scope not_bin A {
   echoe &{.echo $A} &&{.echo $A} $A $nl
   .scope otherwise A {
      echo &{.echo $A} &&{.echo $A} &&&{.echo $A} ${.echo $A} $A}}
.scope not_bin A {echoe &{.echo &A $A} &&{.echo &A &&A} ${.echo &A $A} $nl}
sa &{.echo $A} {echo $args &1$}
sa &{.throw .nop} {}
sa ${.throw .nop} {}
se {e &{.throw .nop}}
se {e &&{.throw .nop}; e after}
se {e &&{.throw .nop}; e after}
.collect_errors_except .nop {
  ${.throw echo exception from inside substitution}
  echo after failed substitution}
.collect_errors_only .failed_substitution echo {
  ${.throw echo exception from inside substitution}
  echo after failed substitution}
.collect_errors_only .nop {
  ${.throw echo exception from inside substitution}
  echo after failed substitution}
echo before .{argfunction} between [.{argfunction}] after
echo before ..{still bad} between [.{missing close} after
echo x{echo bad argfunction style}
echo x&&&{echo x}
echo $+{echo x}
echo &+{echo x}
echo &&${echo x}
echo ${echo 0}
echo &{.echo 0}
.echo nevermore &{/bin/echo quoth the raven} 
sa ${echo $B}$@1 ${echo $B}$1 ${echo $B}XYZ {echo $# $args$}
sa ${echoe $B} {echow $# $args$}
sa &{echoe $B} {echow $# $args$}
sa ${echoe $B}$ {echow $# $args$}
sa &{echoe $B}$ {echow $# $args$}
sa &{.echo $B}$ {echow $# $args$}
se {echow $# &{echow $B}$}
se {echo $# &{echo $B}$}
sa &{echoe $B}$$ {echoe $# $args$ $nl}
sa ${echoe $B}$$ {echoe $# $args$ $nl}
se {sa ${echoe $B}$1 {echoe $# $args$ $nl}}
se {sa &{echoe $B}$1 {.echo $# $args$ $nl}}
se {sa ${echoe $B}$$$1 {echo $# $args$}}
se {sa &{echoe $B}$$$1 {echo $# $args$}}
.unset A
.unset B
sa &{echoe ((zero zero) (one one) two three)}$10 {echo $# $args$}
sa ${echoe (zero zero) \)one one two three}$1 {echo $# $args$}
sa &{echoe (zero zero) \(one one two three}$1 {echo $# $args$}
.scope ${echoe (zero zero) \)one one two three} arg {.echo $# $arg $nl}
c x &{.echo (y y)}$ x $nl
c x ${.echo ( y y )}$ x $nl
c x &{.echo (( y) (y ))}$ x $nl
c x ${.echo (    )}$ x $nl
c x &{.echo (
y
y
)}$ x $nl

# file redirection (but don't overwrite files that exist)
# .for_each_line
/bin/cat <non_existent_file
.for_each_line
.for_each_line <non_existent_file argv {echo line of $# \( $argv$ \)}
se {sa {echo hi >one >two} {cat <three <four}
}
se {
  .if .test_file_exists outfile {echo failed to remove outfile}
  .else {echo outfile properly removed}}
se {echo hi >outfile}
/bin/cat outfile
se {
  .if .test_is_number 0 {>outfile /bin/echo there}
  .else {.nop}}
/bin/cat outfile
se {se >outfile {
  echo line 1; echo line 2 longer; .echo $nl; echo ending}}
/bin/cat <outfile
.for_each_line {}
.for_each_line <outfile x
.for_each_line <outfile <another A{}
.for_each_line <outfile argv ... {
  echo current line $argv$
  if_only .test_greater $2 1 {
    .collect_errors_except .nop {
      .throw .continue
      .throw .break
      .throw .continue}}
  .throw .continue
  echo not printed}
.for_each_line <outfile argv ... {
  echo line of $# \( $argv$ \); .throw echo exception in for_each_line}
.for_each_line <outfile [argv ...] {echo line of $# \( $argv$ \)}
.for_each_line <outfile first second [third] {
  .nop $first;
  if_only .var_exists third {.combine \(extra\ is\  $third \)\ }
  echo $second}
/bin/rm outfile

# soon level promotion .get_max_nesting .set_max_nesting
.get_max_nesting excess {excess argfunction}
.set_max_nesting {excess argfunction}
.global A 0
.global OLD_NESTING ${.get_max_nesting}
.set_max_nesting 46
fn x .{argfunction} {.var_add A 1
     se {.var_add A 1
        se {.var_add A 1
           se {.var_add A 1
              se {.var_add A 1
                 se {.argfunction}}}}}}
.scope 00 A {x {echo &A &&A &&&A $A}}
.scope 00 A {x {x {x {x {echo &A &&A &&&A &&&&A &&&&&A &&&&&&A $A}}}}}
.scope 00 A {
  x {echoe &{.echo &A $A} . &&{.echo &A &&A $A} . &&&{.echo &A &&A &&&A $A} . ${
  .echo &A &&A &&&A $A} . $A $nl}}
.set A 0
x {x {x {x {
  echoe &{.echo &A $A} . &&{.echo &A &&A $A} . &&&{.echo &A &&A &&&A $A} . &&&&{
  .echo &A &&A &&&A &&&&A $A} . &&&&&{.echo &A &&A &&&A &&&&A &&&&&A $A} . ${
  .echo &A &&A &&&A &&&&A &&&&&A $A} . $A $nl}}}}
.rm_executable x
.function not soon {&&&enough}
.set_max_nesting $OLD_NESTING
.unset A
.unset OLD_NESTING

## builtin tests
# .argc
whence .argc
.argc {excess argfunc}
.scope () {.argc; .echo $nl}
.scope () {.argc (); .echo $nl}
.scope () {.argc (1 a) (2 b (c d)) (3 ((e () f))); .echo $nl}

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
  .combine $nl}

# .combine
.combine
.combine something {excess argfunc}
sa on () e \ two ( ) {.combine $args$ ${.argc $args$} $nl}

# .echo .error
.error
.error something {excess argfunc}
.nop .error cannot test error with a diff until you can redirect it $nl
.echo
.echo something {excess argfunc}
.echo these are fixed strings $nl

# .exec .fork Binary Old_argv_t .binary_not_found
.fork
.fork echo text
.fork .exit 127
.fork sa 126 {sa $args$ {echo about to return $args$; .exit $args$}}
.exec
.exec something {excess argfunc}
.exec /bin/ech does not exist
.exec /etc/rwshrc config files are not interpreters
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
.throw sa .throw {echo even from $args$ 7 is a number}
.fallback_handler .throw sa .throw {echo even from $args$ 7 is a number}
.throw .throw sa {echo even from $args$ 7 is a number}

# .for
.for q {echo no arguments $q}
.for no_argfunction q
.for 1 q {echo one argument $q}
.for 1 2 q {echo one argument $q; .throw echo exception in for}
.for 1 2 3 4 q {
  echo current arg $q
  if_only .test_greater $q 2 {
    .collect_errors_except .nop {
      .throw .continue
      .throw .break
      .throw .continue}}
  .throw .continue
  echo not printed}
fn fIJ outer inner {
  .local sum 0
  .nop $sum
  $outer 1 2 3 4 5 6 I {
    if_only_not .test_string_equal $I 1 {.combine $nl}
    .combine $I :
    $inner 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 J {
      if_only .test_less 1 $J {.echo ,}
      .echo $J
      .set sum ${+ $I $J}
      .if .test_less 5 $sum$ {.throw outer_break}
      .else_if .test_less 3 $sum$ {.throw outer_continue}
      .else {}}}
  .echo $nl}
fIJ .for .for
fIJ outer_for .for
fIJ .for outer_for
fIJ outer_for outer_for
.for 1 2 3 4 for_argument {echoe four arguments $for_argument $nl}
.for (-x -y permitted) (-z rejected) ([-x] [-y arg] [-z]) {echo $-*}

# .function .rm_executable .list_locals
whence .function
.function
.rm_executable {excess argfunction}
.list_locals excess {excess argfunction}
.function .exit {.nop}
.function .escaped_argfunction {.nop}
.rm_executable a
.function a {.nop}
.whence_function a
a 1 2 3
.rm_executable a
.whence_function a
.global A \
sa () A () A () {.echo $args$ ${.argc $args$} $nl}
.function a arg {echoe 9 $A $arg @/usr $nl}
.whence_function a
a \
a 1
a 1 2
.function a [args ...] {.nop $args$; echoe $*5 $* $*0 $nl}
a
a 1
a 1 2
fn g name .{argfunction} {.function $name name {
  .function $name {.argfunction}}}
g a {echoe 3 2 1 $nl}
whence a
a b
b
# a function redefining itself doesn't seg fault
fn g {echoe hi $nl; fn g {echoe there $nl}; fn h {echoe nothing here}; g}
g
fn ll {.list_locals; .combine $nl}
.function .exit {echoe cannot redefine a builtin as a function}
.function .a {can define a function for non-existant builtin}
.function .argfunction {echoe cannot define .argfunction}
.function a y y {echoe illegal duplicate required parameter}
.function a [-x] [-x] {echoe illegal duplicate flag parameter}
.function a [x x] {echoe illegal duplicate optional parameter}
.function a [-x arg bar] [-y arg] {echoe illegal duplicate flag argument}
.function a -x [-x] {echoe evil duplication between flags positional}
.function a -- -- {echoe -- cannot be a duplicate parameter}
.function a [--] [--] {echoe [--] cannot be a duplicate parameter}
.function a [--] -- {echoe -- and [--] cannot both be parameters}
.function a [-- arg] {echoe -- cannot take arguments}
.function a [arg -- foo] {echoe -- cannot take arguments}
.rm_executable nonsense
whence test_var_greater
.scope 5 n {test_var_greater n}
.scope 5 n {test_var_greater n 3 12}
.scope 5 n {test_var_greater n 3}
whence ntimes
ntimes -- 3 {echoe $n remaining $nl}
ntimes 2 {ntimes 3 {echoe &&n and $n remaining $nl}}
.function a [-x] [-] [--long-opt y second {
  echoe mismatched bracket (i.e. missing close brakcet)}
.function a [-?] [--] {.list_locals}
.function a [-x] [--] foo {.list_locals}
.function a [-?] -- foo {.list_locals}
.function a [-x] -- {.list_locals}
.function a [--] {if_only .var_exists -- {.echo $--}; ll}
whence a
a
a --
a foo
.function a [-x] [--long-opt y] second {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a
a single
a -x single
a --long-opt arg single
a --long-opt single
a --long-opt first -x --long-opt second single
.function a [-q option1 option2] [-x o1 o2 o3 o4] required {
  forj ${ll}$ {.combine $j \( $$j \) \ }; .echo $nl}
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
.function a [optional0] -- [optional1 optional2 optional3] required {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a
a single
a one two
a one two three
a one two three four five
a one two three four five six seven eight nine
.function a -y [second] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a
a 1
a 1 2
a 1 2 3
.function a [-x] -y [--long-opt] second [--] [-] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a --long-opt -xx over-long flag
a -xx --long-opt over-long flag extra excess
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
a -x just flags-x --long-opt -x
a -x just flags-x -- --long-opt
a -x just flags-x --other --also-wrong
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
.function a [-first] [-to] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
  echo nothing_required}
whence a
a
a excess
a -to
a -first --
a -first excess
a -to -- -first -- stops flag parsing rather than being a flag
a -to -first
.function a [-?] [-first] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
  echo nothing_required}
whence a
a
a excess
a -to
a -first --
a -first excess
a -to -- -first
a -to -first
.function a [-*] [-first] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
  echo nothing_required}
whence a
a
a -to -- -first
a -to -first
.function a [-?] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
  echo nothing_required}
whence a
a
.function a ... y {}
.function a [-x] [--file file] ... y {}
.function a [...] x {}
.function a [... y] x {}
.function a x ... [y z] {}
.function a x [... y] z ... {}
.function a [y z] ... x {}
.function a [y] ... x {}
whence a
.function a x ... y ... {}
.function a [x ...] [y z] {}
.function a [x ...] y ... {}
.function a [x ... a] [y] {}
.function a [x ... a] y ... {}
.function a [x ... a ...] {}
.function a [x ... a ... b] {}
.function a [-x ...] b [c] {}
.function a -? x ... y {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
.function a -* x ... y {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
.function a [-?] x ... y {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a -c -a -b first second third
a -c first -a second -b third
a 
a first
a first (se cond)
a first (se cond) third
a first (se cond) third fourth (fi fth)
.function a [-?] x [...] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a
a first second third fourth fifth
.function a [-?] x [--] [y ...] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a
a first
a first second
a first second third
a first second third fourth fifth
.function a [-?] a [b ... c] d {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a first
a first second
a first second third
a first second third fourth
a first second third fourth fifth
a first second third fourth fifth sixth
.function a [-x ...] b c {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a first second
a -x first
a -x first second
a -x first second third
a -x first second third fourth
.function a [-x b ...] c {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a first
a -x first
a -x (fi rst) second
a -x first (sec ond) third
a -x first -x (sec ond) third
a -x first -y (sec ond) third
a -x (fi rst) (sec ond) third fourth
.function a [-x ... b] c {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
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
.function a x [-?] [... y z] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
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
.function a [-?] [x] [... y] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a
a (fi rst)
a (fi rst) second
a first (sec ond) third
a (fi rst) (sec ond) third fourth
a (fi rst) (sec ond) (thi rd) (fou rth) (fi fth)
.function a [-?] [x y] [... z] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
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
.function a -- [-x] y z {}
.function a -- [x y] z {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }; .echo $nl}
whence a
a
a --file
a first second third
a -x second -

# .get_max_collectible_exceptions .set_max_collectible_exceptions
# .collect_errors_except .collect_errors_only
.get_max_collectible_exceptions excess
.set_max_collectible_exceptions
.get_max_collectible_exceptions {excess}
.set_max_collectible_exceptions 1 {excess}
.set_max_collectible_exceptions NaN
.scope () {.get_max_collectible_exceptions; .echo $nl}
.set_max_collectible_exceptions 0
.scope () {.get_max_collectible_exceptions; .echo $nl}
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
.collect_errors_except echo {${.throw echo exception thrown directly}}
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
.function collecting_handler args ... {
  echo $args$
}
.scope () {.get_max_extra_exceptions; .echo $nl}
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
.scope () {.get_max_collectible_exceptions; .echo $nl}

# .global .local .local_declare .unset .var_exists
.global
.global x y z
.global x y {excess argfunc}
.local
.local x y z
.local x y {excess argfunc}
.local_declare
.local_declare x {excess argfunc}
.unset
.unset x {excess argfunc}
.unset x y
.unset FIGNORE
.var_exists
.var_exists x {excess argfunc}
.global 100 nihilism
.local 0 nihilism
.local_declare 0 #
.global .var_exists (must be requested to be checked)
echo $.var_exists$
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
.local_declare x
.var_exists x
.var_exists x y
echo $x
.global x nihilism
.global x ubernihilism
.global i nihilism
.unset i
.global j nihilism
fn echo-i-y in-out func {
  if_only .var_exists i {echo $in-out $func i $i}
  if_only .var_exists j {echo $in-out $func j $j}
  if_only .var_exists k {echo $in-out $func k $k}
  if_only .var_exists m {echo $in-out $func m $m}
  if_only .var_exists n {echo $in-out $func n $n}
  if_only .var_exists x {echo $in-out $func x $x}
  if_only .var_exists y {echo $in-out $func y $y}}
fn a {echo-i-y in a
      .local x (first level not global)
      .local y (level one not global)
      .local z (will be unused)
      .local_declare i j k l m n
      .set i (declared first level not global)
      .set j (declared first level not global)
      .set k (declared level one not global)
      .set l (will be unused)
      .set n (will be used)
      b
      echo-i-y out a}
fn b {echo-i-y in b
      .local x (second level masks first)
      .set y   (level two overwrites one)
      .local_declare i j m
      echo undefined i will hide the first level local
      .set j (declared second level masks first)
      .set k (declared level two overwrites one)
      c
      echo-i-y out b}
fn c {echo-i-y in c
      .reinterpret .reinterpreted .reinterpreted (n [j] y)
      echo-i-y middle c
      # can unset a local, but won't remove everything named x
      .unset x
      .unset j
      .unset n
      .try_catch_recursive .global_would_be_masked {
        .global y (attempting to create global masked by local)}
      .set x (third level overwrites first)
      .set i (declared third level overwrites second)
      .set j (declared third level overwrites first)
      .local x (third level masks first)
      .local_declare i j m
      .set i (declared third level masks first)
      .set j (declared third level masks first)
      .set y (level three overwrites one)
      .set k (declared level three overwrites one)
      echo-i-y out c}
a
# demonstrating that final values are not retained
a
ntimes 2 {
  echo before $x$ , $j$
  .local_declare x
  .local_declare x
  .local j (locally defined)
  echo between $x$ , $j$
  .set x locally set
  echo after $x$ , $j$}
echo $x
.var_exists y
.unset x
.unset i
.unset j
.var_exists x

# .store_output
.store_output x
.store_output {echo some text}
.store_output # {echo some text}
.store_output x {echo some text}
.global x ()
.store_output x {echo some text; .throw echo exception}
echoe $x
.store_output x {echo some text}
echoe $x
.unset x

## if_core
# helper functions
.function silent_throw [text ...] {
  .collect_errors_only .false echo {
    .throw .false first
    .throw echo exception $text$
    .throw .false second
    .nop}}
.function caught_silent_throw [text ...] {
  .try_catch_recursive echo {silent_throw $text$}}
silent_throw on its own
caught_silent_throw on its own
fn if_true .{argfunction} {.if .test_not_empty ( ) {.argfunction}}
fn if_false .{argfunction} {.if .test_not_empty ()  {.argfunction}}
fn else_if_true .{argfunction} {
  .else_if .test_string_unequal q w {.argfunction}}
fn else_if_false .{argfunction} {
  .else_if .test_string_unequal q q {.argfunction}}
fn else_if_not_true .{argfunction} {
  .else_if_not .test_string_equal x x {.argfunction}}
fn else_if_not_false .{argfunction} {
  .else_if_not .test_string_equal y z {.argfunction}}
fn obscured_else .{argfunction} {.scope () {.scope () {.else {.argfunction}}}}
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
.function !x args ... {
  .if .throw .false anyway {}
  .else_if_not $args$ {.nop}
  .else {.throw .false ${.echo ! $args}}}
.function !! args ... {
  .if $args$ {}
  .else {.throw .false ${.echo !! $args}}}
.test_string_equal 42 420e-1
.test_number_equal 42 420e-1
!  .test_greater 50 230
!x .test_greater 50 230
!! !  .test_greater 50 230
!  !! .test_greater 50 230
!! .test_greater 50 230
!! !! .test_greater 50 230
!  .test_less 5 2.3e1
!x .test_less 5 2.3e1
!! !x .test_less 5 2.3e1
!x !! .test_less 5 2.3e1
!! .test_less 5 2.3e1
!! !! .test_less 5 2.3e1
!  !  .test_executable_exists !!!
!! !  !  .test_executable_exists !!!
!  !! !  .test_executable_exists !!!
!  !  !! .test_executable_exists !!!
!  !x .test_executable_exists !!!
!! !  !x .test_executable_exists !!!
!  !! !x .test_executable_exists !!!
!  !x !! .test_executable_exists !!!
!x !  .test_executable_exists !!!
!! !x !  .test_executable_exists !!!
!x !! !  .test_executable_exists !!!
!x !  !! .test_executable_exists !!!
!x !x .test_executable_exists !!!
!! !x !x .test_executable_exists !!!
!x !! !x .test_executable_exists !!!
!x !x !! .test_executable_exists !!!
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
se {.if .test_string_equal 42 420e-1 {echo if}; .else {echo else}}
se {.if .test_number_equal 42 420e-1 {echo if}; .else {echo else}}
se {.if !  .test_greater 50 230 {echo if}; .else {echo else}}
se {.if !x .test_greater 50 230 {echo if}; .else {echo else}}
se {.if !! !  .test_greater 50 230 {echo if}; .else {echo else}}
se {.if !  !! .test_greater 50 230 {echo if}; .else {echo else}}
se {.if !! .test_greater 50 230 {echo if}; .else {echo else}}
se {.if !  .test_less 5 2.3e1 {echo if}; .else {echo else}}
se {.if !x .test_less 5 2.3e1 {echo if}; .else {echo else}}
se {.if !! !x .test_less 5 2.3e1 {echo if}; .else {echo else}}
se {.if !x !! .test_less 5 2.3e1 {echo if}; .else {echo else}}
se {.if !! .test_less 5 2.3e1 {echo if}; .else {echo else}}
se {.if !  !  .test_executable_exists !!! {echo if}; .else {echo else}}
se {.if !  !x .test_executable_exists !!! {echo if}; .else {echo else}}
se {.if !x !  .test_executable_exists !!! {echo if}; .else {echo else}}
se {.if !x !x .test_executable_exists !!! {echo if}; .else {echo else}}
se {.if !  !  .whence_function !!! {echo if}; .else {echo else}}
se {.if !x !x .whence_function !!! {echo if}; .else {echo else}}
se {.if !  !  .test_not_empty 0 {echo if}; .else {echo else}}
se {.if !  !x .test_not_empty 0 {echo if}; .else {echo else}}
se {.if !x !  .test_not_empty 0 {echo if}; .else {echo else}}
se {.if !x !x .test_not_empty 0 {echo if}; .else {echo else}}
se {.if !  !  !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !  !  !  .test_in 0 {echo if}; .else {echo else}}
se {.if !  !  !  !  .test_in 0 {echo if}; .else {echo else}}
se {.if !  !  !x !  .test_in 0 {echo if}; .else {echo else}}
se {.if !  !x !  !  .test_in 0 {echo if}; .else {echo else}}
se {.if !  !x !x !  .test_in 0 {echo if}; .else {echo else}}
se {.if !x !  !  !  .test_in 0 {echo if}; .else {echo else}}
se {.if !x !  !x !  .test_in 0 {echo if}; .else {echo else}}
se {.if !x !x !  !  .test_in 0 {echo if}; .else {echo else}}
se {.if !x !x !x !  .test_in 0 {echo if}; .else {echo else}}
se {.if !  !  !  !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !  !  !x !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !  !x !  !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !  !x !x !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !x !  !  !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !x !  !x !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !x !x !  !  .test_in 0 0 {echo if}; .else {echo else}}
se {.if !x !x !x !  .test_in 0 0 {echo if}; .else {echo else}}

# properly sequenced un-nested conditionals where everything succeeds
se {
  if_true {echo if_true printing}
  else_if_true {echo else_if_true not printing when condition true}
  else_if_not_false {echo else_if_not_false not printing when condition true}
  else_if_false {echo else_if_false not printing when condition true}
  else_if_not_true {echo else_if_not_false not printing when condition true}
  obscured_else {echo obscured_else not printing when condition true}}
se {
  if_false {echo if_false not printing}
  else_if_false {echo else_if_false not printing when condition false}
  else_if_not_true {echo else_if_not_true not printing when condition false}
  obscured_else {echo obscured_else is printing when condition false}}
se {
  if_false {echo if_false not printing}
  else_if_true {echo else_if_true printing when condition false}
  obscured_else {echo obscured_else not printing when condition true}}
se {
  if_false {echo if_false not printing}
  else_if_not_false {echo else_if_not_false printing when condition false}
  obscured_else {echo obscured_else not printing when condition true}}
se {
  if_with_body .test_not_empty a
  else_if_with_body .test_not_empty a
  else_with_body}
se {
  if_with_body .test_not_empty ()
  else_if_with_body .test_not_empty ()
  else_with_body text expanded}
se {
  if_with_body .test_not_empty ()
  else_if_with_body .test_not_empty a
  else_with_body text}

# nested conditionals
se {
  .if conditional_true {echo conditional_true printing}
  .else {echo else not printing when condition true}}
se {
  .if conditional_false {echo conditional_false not printing}
  .else {echo else is printing when condition false}}
se {
  .if ${conditional_echo .test_is_number 71}$ {echo conditional_echo printing}
  .else {echo else not printing when condition true}}
se {
  .if ${conditional_echo .test_is_number pi}$ {echo conditional_echo not printing}
  .else {echo else is printing when condition false}}
se {
  if_true {echo ${conditional_echo print this}}
  .else {echo else not printing when condition true}}
se {
  if_false {echo ${conditional_echo .test_not_empty ()}}
  .else {echo else is printing when condition false}}
se {
  .if conditional_true {echo ${conditional_echo print this}}
  .else {echo ${conditional_echo not printing}}}
se {
  .if conditional_false {echo ${conditional_echo not printing}}
  .else {echo ${conditional_echo print this}}}

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
if_false {echo if block not closed}
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
se {
  .if ${conditional_echo unfindable anywhere}$ {echo also a failed substitution}
  .else {echo else skipped when condition threw an exception}}
se {
  if_true {echo &&&{unfindable anywhere}}
  .else {echo else not printing when condition true and if body threw}}
if_true {echo &{unfindable anywhere}}
.else {echo else throws an exception because we never got into the if}
if_only .nop {.for .break ex {if_only .nop {.throw $ex properly nested}}}

# .if .else_if .else_if_not .else
.collect_errors_except .nop {
  .if
  whence .if
  .else_if .test_is_number 0 {echo do not run after an exception}
  whence .else_if
  .else_if_not .test_is_number false {echo do not run after an exception}
  .else_if
  whence .else_if_not
  .else_if_not
  whence .else
  .else {echo do not run after an exception}}
.if missing argfunction
.else_if missing argfunction
.else_if_not missing argfunction
.else
se {
  .if .test_is_number 0 {}
  .else {echo first else for if}
  .else {echo second else for if}}
se {
  .if .test_is_number 0 {echo if true}
  .else {echo else 0 not a number}}
se {
  .if .test_is_number false {echo if false}
  .else {echo else false is not a number}}
.else {}
.else_if .test_is_number 0 {echo not this one}
.else {}
.else_if_not .test_is_number false {echo nor this}

# .internal_features .internal_functions .internal_vars
.internal_features 1 {excess argfunc}
.internal_functions 1
.internal_functions {excess argfunc}
.internal_vars 1 {excess argfunc}
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
forj ${.list_executables}$ {.combine $j $nl}

# .ls
.ls
.ls /bin {excess argfunc}
.ls /bin/usr/
.ls /bi  /us /nr
.ls /bin /usr/

# .nop
whence .nop
.nop
.nop {optional argfunc}
.nop 1 2 3 4 5

# .replace_exception
.replace_exception
.replace_exception echo not in exception handler
.try_catch_recursive .replace_exception {
  .throw .replace_exception echo now in exception handler}
.try_catch_recursive .replace_exception {
  .throw .replace_exception forj {echo $j is on the call stack}}

# .scope .reinterpret prototype.cc
.scope
.reinterpret {e $foo}
.scope () {.scope foo}
.scope a (y y) {echo illegal duplicate required parameter}
.scope a ([-x] [-x]) {echo illegal duplicate flag parameter}
.scope a ([x x]) {echo illegal duplicate optional parameter}
.scope a ([-x arg bar] [-y arg]) {echo illegal duplicate flag argument}
.scope a (-x [-x]) {echo evil duplication between flags positional}
.scope -- -- {echo -- as a prototype forbids even -- as arguments}
.scope -- () {echo but ${.list_locals} is acceptable for empty prototype}
.scope () {echo no arguments are also good for empty prototype}
.scope a (-- --) {echo -- cannot be a duplicate parameter}
.scope a ([--] [--]) {echo [--] cannot be a duplicate flag parameter}
.scope a ([--] --) {echo -- and [--] cannot both be parameters}
.scope a ([-- arg]) {echo -- cannot take arguments}
.scope a ([arg -- foo]) {echo -- cannot be an argument}
.scope (args ...) {foreok ${.list_locals}$ j {.combine $j = $$j $nl}}
.scope ([args ...]) {foreok ${.list_locals}$ j {.combine $j = $$j $nl}}
.scope () (args ...) {foreok ${.list_locals}$ j {.combine $j = $$j $nl}}
.scope () ([args ...]) {foreok ${.list_locals}$ j {.combine $j = $$j $nl}}
.scope -x -y a b ([-?] args ...) {
  forj ${.list_locals}$ {.combine $j = $$j \ }; .echo $nl}
.scope .foo {echo fixed arguments not yet supported}
.scope a ([-? bad] arg) {e -? cannot currently take arguments}
.scope a ([-* bad] arg) {e -* (aka -?) cannot currently take arguments}
.scope -a -* -b a ([-?] a) {
  forj ${.list_locals}$ {.combine $j = $$j \ }; .echo $nl}
.scope bar foo {echo aa $foo bb}
.scope baz bax (foo bar) {
  forj ${.list_locals}$ {.combine $j = $$j \ }; .echo $nl}
.scope foo bar baz bax (args ...) {echo aa $args$2 bb $args$1 cc}
.scope single ([-x] [--long-opt y] second) {
  var_val ${.list_locals}$; .echo $nl}
.reinterpret [jj]
.scope [-x] {.reinterpret -x bar ([-x kk])}
.scope ([n y]) {.scope local_val x {.reinterpret insufficient (n y)}}
.scope one two (a b [c]) {
  .reinterpret $a $b (c [b] a)
  echo $a$ $c$}
fn arg_req .{argfunction} {
  echo before argfunction
  .scope () {.argfunction}}
arg_req {echo the argfunction}
arg_req not an argfunction
fn arg_opt [.{argfunction}] {
  .test_executable_exists .argfunction {.argfunction}
  .scope () {.argfunction}}
arg_opt {echo the argfunction}
arg_opt
arg_opt not an argfunction
fn arg_none {echo argfunction will not be accepted}
arg_none bad arguments {echo will not print}
arg_none
.function a [-x] [--long-opt y] second {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
  .combine $nl}
whence a
.function pt -- args ... {
  .scope $args$ ([-x] [--long-opt y] second) {
    forj ${.list_locals}$ {.combine $j \( $$j \) \ }}
  .combine $nl
  .scope $args$ ( [-?] [--long-opt y] second) {
    forj ${.list_locals}$ {.combine $j \( $$j \) \ }}
  .combine $nl}
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
.function a [-?] [-first] {
  forj ${.list_locals}$ {.combine $j \( $$j \) \ }
  echo nothing_required}
whence a
.function pts -- [args ...] {
  .if var_exists args {
    .scope $args$ ([-first] [-?]) {
      forj ${.list_locals}$ {.combine $j \( $$j \) \ }}
      echo nothing_required}
  else {.scope ([-first] [-?]) {
      forj ${.list_locals}$ {.combine $j \( $$j \) \ }}
      echo nothing_required}}
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

# .selection_set
.selection_set A
.selection_set A /usr {excess argfunc}
.selection_set A /usr
echo $A$
.selection_set A ./
echo $A$
.selection_set A local/include
echo $A$
.selection_set A ..
echo $A$
.selection_set A ()
echo $A$
.selection_set A ./local/../../bin
echo $A$
.selection_set A sbin etc
echo $A$

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
echo $A

# .try_catch_recursive .get_max_extra_exceptions .set_max_extra_exceptions
.try_catch_recursive .function_not_found
.try_catch_recursive {.test_less 0 A}
fn e_after .{argfunction} {se {.argfunction}; echo after}
e_after {.try_catch_recursive .not_a_number .function_not_found {
  .test_less 0 A}}
e_after {
  .try_catch_recursive .function_not_found .binary_not_found \
                       .failed_substitution {
    ..test_less 0 A}}
e_after {.try_catch_recursive .not_a_number {.test_less 0 A}}
e_after {.try_catch_recursive .not_a_number {.cho A}}
e_after {.try_catch_recursive .not_a_number .function_not_found {echo A}}
e_after {sa echo hi {.try_catch_recursive ${.internal_functions}$ {&&&args$}}}
.get_max_extra_exceptions excess
.set_max_extra_exceptions
.get_max_extra_exceptions {excess}
.set_max_extra_exceptions 1 {excess}
.set_max_extra_exceptions NaN
.set_max_extra_exceptions -1
.scope () {.get_max_extra_exceptions; .echo $nl}
.set_max_extra_exceptions 0
.scope () {.get_max_extra_exceptions; .echo $nl}
e_after {.try_catch_recursive .not_a_number {
  .try_catch_recursive .not_a_number {.test_less 0 A}}}
e_after {.try_catch_recursive .not_a_number {
  .try_catch_recursive .not_a_number {.test_less 0 A}}}
e_after {sa echo hi {.try_catch_recursive ${.internal_functions}$ {&&&args$}}}
.set_max_extra_exceptions 5
.scope () {.get_max_extra_exceptions; .echo $nl}

# .stepwise
.function wrapper args ... {a $args$ two; a $args$ three}
.function a args ... {echow $args$ one; echow $args$ two
  echow $args$ three}
.function d args ... {echow $args$; .stepwise $args$ (cmd ...) {d $cmd$}}
.stepwise {echo $argv$}
.stepwise wrapper 1 2 (cmd ...)
.stepwise stepwise (cmd ...) {echo $cmd}
.stepwise .stepwise (cmd ...) {echo $cmd}
.stepwise ! (cmd ...) {echo $cmd}
.stepwise wrapper 1 2 (cmd ...) {echo $cmd$}
.function wrapper args ... {a $args$ one
  a $args$ two; a $args$ three}
wrapper 1 2
.stepwise wrapper 1 2 (cmd ...) {d $cmd$}
.stepwise wrapper 1 2 (argv ...) {
  echo current line: $argv$
  if_only .test_string_equal $4 two {
    .collect_errors_except .nop {
      .throw .continue
      .throw .break
      .throw .continue}}
  .throw .continue
  echo not printed}
.stepwise wrapper 1 2 (argv ...) {echoe $argv$ $nl}
.function a {echo -x allowed; echo -y allowed; echo -z rejected}
.stepwise a (cmd [-x] [-y] comment) {$cmd $-* $comment}

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
.test_not_empty x ()
.test_not_empty () x
.test_not_empty () () ()
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
.test_greater 6.022e23c 6.022e23e
.test_greater 6.022e9000000000 .6022e23
.test_greater 6.022e23 .6022e9000000001
.test_greater 6.022e2 6.022e23
.test_greater 6.022e23 .6022e24
.test_greater 6.022e23 6.022e2
.test_less 6.022e23
.test_less 6.022b23 6.022e23 {excess argfunc}
.test_less 6.022b23 6.022a23
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
.function wm [args ...] .{argfunction} {
   .whence_function .mapped_argfunction {.argfunction}
   .scope $args$ (a1 a2 a3) {.argfunction}}
wm (aa ab ac) bb cc {
  echoe x &&a1 &&2 $a3 y &&a1$1 z &&a1$2; .nop $a2
  sa &&a1$ {echoe () w $args$ $#; echoe $nl}}
.scope (aa ab ac) bb cc (args more ...) {.scope $args $more$ (args more ...) {
  sa &&1 {echo $args$ $#}
  sa &&args {echo $args$ $#}
  sa &&&args {echo $args$ $#}
  sa $args {echo $args$ $#}
  sa $args$ {echo $args$ $#}
  sa $more$ {echo $args$ $#}}}
.function wm [args ...] .{argfunction} {
   whence .mapped_argfunction {.argfunction}
   .nop $args
   se {.argfunction}
   .echo $nl}
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

# .usleep .execution_count .last_execution_time .total_execution_time
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
.scope () {
  .execution_count .mapped_argfunction {echo not tracked}; .echo $nl}
.scope () {
  .execution_count .usleep; .echo $nl}
.last_execution_time
.last_execution_time j
.scope () {
  .last_execution_time .mapped_argfunction {echo not tracked}; .echo $nl}
# .last_execution_time .usleep
.total_execution_time
.scope () {
  .total_execution_time .mapped_argfunction {echo not tracked}; .echo $nl}
.total_execution_time j
# .total_execution_time .usleep

# .which_path
echo ${.which_path cat}
echo ${.which_path cat /bin {excess argfunc}}
echo ${.which_path cat \ }
echo ${.which_path does<not>exist /bin:/usr/bin}
echo ${.which_path cat :/bin:/usr/bin}
echo ${.which_path cat /usr/bin/:/bin/:}
echo ${.which_path /bin/cat /usr/bin/:/bin/:}
echo ${.which_path rwsh /usr/bin:/bin}
echo ${.which_path rwsh .:/usr/bin:/bin}
echo ${.which_path rwsh /usr/bin:/bin:.}
echo ${.which_path ./rwsh /usr/bin:/bin}
echo ${.which_path ./rwsh /bin:.}
echo ${.which_path ../bin/rwsh /usr/bin:/bin}
echo ${.which_path rwsh :/usr/bin::/usr/bin:}
echo ${.which_path rwsh /usr/bin:.}

# .while
.while {e ARGS}
.while var_less A 4
.scope 0 A {.while var_less A 4 {echo printed; .throw .break; echo skipped}}
.scope 0 A {.while var_less A 4 {echo printed; .set A 4}}
.scope 4 A {.while var_less A 4 {echo skipped}}
.while .nop {.throw echo exception within while}
.while silent_throw within condition {e body skipped}
.while .throw .false because {echo body skipped}
.scope 0 A {.while .throw .break {echo condition cannot break}}
.scope 0 A {.while .throw .continue {echo condition cannot continue}}
.scope 0 A {.while var_less A 4 {echo body can break; .throw .break}}
.scope 0 A {.while var_less A 4 {echoe in .while argfunction $A $nl; .var_add A 1}}
.scope 0 A {.while var_in A 1 2 {echo A is $A; .var_add A 1}}
.scope 0 A {do_while var_in A 1 2 {echo A is $A; .var_add A 1}}
.scope 1 A {.while var_in A 1 2 {echo A is $A; .var_add A 1}}
.scope 1 A {while_and_one_more var_in A 1 2 {echo A is $A; .var_add A 1}}
fn throw_the_kitchen_sink cmd ... {
  $cmd$ {
    echo early in argfunction $A
    .var_add A 1
    if_only .test_less $A 3 {.throw .continue}
    .collect_errors_except .nop {
      .throw .continue
      .throw .break
      .throw .continue}
    echo late in .while argfunction $A}
  echo after $cmd$0
  .throw .break
  echo do not echo after a .break thrown outside control flow}
.scope 0 A {throw_the_kitchen_sink .while var_less A 4}
.scope 0 A {throw_the_kitchen_sink do_while var_less A 4}
.scope 3 A {throw_the_kitchen_sink do_while var_less A 6}
.scope 0 A {throw_the_kitchen_sink while_and_one_more var_less A 4}
.function tf N {.test_less $A $N}
.scope 0 A {.while tf 4 {
  .if .test_number_equal 0 $A {.set A 1}
  .else {.function tf N {.throw .false $N}}
  echo in overwriting argfunction}}
.set_max_nesting 30
fn IJK outer middle inner {
  .scope 0 I {
    .local sum 0
    .nop $sum
    $outer var_less I 5 {
      .var_add I 1
      .scope 0 J {
        $middle var_less J 2 {
          .var_add J 1
          if_only_not .test_string_equal ${e $I $J} (1 1) {.combine $nl}
          .combine $I . $J :
          .scope 0 K {
            $inner var_less K 30 {
              .var_add K 1
              if_only .test_less 1 $K {.echo ,}
              .echo $K
              .set sum ${+ $I + $J $K}
              .if .test_less 5 $sum$ {.throw outer_break}
              .else_if .test_less 4 $sum$ {.throw outer_continue}
              .else {}}}}}}}
  .echo $nl}
IJK      .while      .while      .while
IJK outer_while      .while      .while
IJK      .while outer_while      .while
IJK      .while      .while outer_while
IJK outer_while outer_while outer_while
.set_max_nesting 20

fn var_op_val_echo -- var_val op val {
  .scope $var_val (-- var) {.collect_errors_except .nop {
    $op var $val
    echo $var}}}

# .var_add
.var_add
.var_add A 1 2 {excess argfunc}
.var_add B B
.scope [undefined] {.var_add undefined 1}
var_op_val_echo A .var_add 42C
var_op_val_echo NaN .var_add nan
var_op_val_echo 3000000000 .var_add 2
var_op_val_echo 3000000000 .var_add 0
var_op_val_echo 1e309 .var_add -1e309
var_op_val_echo -1e308 .var_add -1e308
var_op_val_echo -1e308 .var_add 1e308
var_op_val_echo 0 .var_add 1e308
var_op_val_echo 1e+308 .var_add 1e308
var_op_val_echo 1e+308 .var_add -1e308
var_op_val_echo 0 .var_add \

# .var_divide
.var_divide A
.var_divide A 1 2 {excess argfunc}
.var_divide B B
.scope [undefined] {.var_divide undefined 1}
var_op_val_echo A .var_divide 42C
var_op_val_echo NaN .var_divide nan
var_op_val_echo 1e3000000000 .var_divide 2e3000000000
var_op_val_echo 1e3000000000 .var_divide 0
var_op_val_echo 16.8 .var_divide 4.2
var_op_val_echo 1.8e-20 .var_divide 1e308
var_op_val_echo 0 .var_divide 3
var_op_val_echo 3 .var_divide 1
var_op_val_echo 9.9999995e-1 .var_divide 1.0000001

# .var_modulo
.var_modulo A
.var_modulo A 1 2 {excess argfunc}
.var_modulo B B
.scope [undefined] {.var_modulo undefined 1}
var_op_val_echo A .var_modulo 42C
var_op_val_echo NaN .var_modulo nan
var_op_val_echo 1e15 .var_modulo 0
var_op_val_echo 16.8 .var_modulo 4.2
var_op_val_echo 1.8e30 .var_modulo 1.000000001
var_op_val_echo 0 .var_modulo 3
var_op_val_echo 3 .var_modulo 1
var_op_val_echo 257 .var_modulo 8
var_op_val_echo 2.55e2 .var_modulo 8.000
var_op_val_echo -5 .var_modulo 3
var_op_val_echo 6 .var_modulo -4

# .var_multiply
.var_multiply A
.var_multiply A 1 2 {excess argfunc}
.var_multiply B B
.scope [undefined] {.var_multiply undefined 1}
var_op_val_echo A .var_multiply 42C
var_op_val_echo NaN .var_multiply nan
var_op_val_echo 1e3000000000 .var_multiply 2e3000000000
var_op_val_echo 1.6 .var_multiply 1.6
var_op_val_echo 1.8e-20 .var_multiply 1e308
var_op_val_echo -1e308 .var_multiply -1e308
var_op_val_echo 1e308 .var_multiply -1e308
var_op_val_echo 0.00 .var_multiply 1e308
var_op_val_echo 0 .var_multiply 3
var_op_val_echo 3 .var_multiply 1
var_op_val_echo 9.9999995e-1 .var_multiply 9.9999995e-1

# .var_subtract
.var_subtract
.var_subtract A 1 2 {excess argfunc}
.var_subtract B B
.scope [undefined] {.var_subtract undefined 1}
var_op_val_echo A .var_subtract 42C
var_op_val_echo NaN .var_subtract nan
var_op_val_echo 3000000000 .var_subtract 2
var_op_val_echo 3000000000 .var_subtract 0
var_op_val_echo 1e308 .var_subtract -1e308
var_op_val_echo -2147483648 .var_subtract 3000000000
var_op_val_echo -5.14748e+09 .var_subtract -2147483648
var_op_val_echo -3e+09 .var_subtract 2147483647
var_op_val_echo -5.14748e+09 .var_subtract 2147483647
var_op_val_echo -7.29496e+09 .var_subtract 2147483647
var_op_val_echo -9.44244e+09 .var_subtract -2147483648
var_op_val_echo -7294960000 .var_subtract \

# .version .version_compatible
.version 1.0
.version {excess argfunc}
.version_compatible
.version_compatible 1.0 1.0
.version_compatible 1.0 {excess argfunc}
echo ${.version}
.version_compatible 1.0
.version_compatible 0.3+

# internal functions
# .after_command .raw_command .prompt
# all of these are used as part of the test itself, though no longer in a
# significant way.
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
se {not_a_thing; e should not be printed}
.rm_executable .function_not_found
.whence_function .function_not_found
whence not_a_thing
not_a_thing
.whence_function .function_not_found

# .help .internal_error
.help
.internal_error techincally this is untestable

# .mapped_argfunction .argfunction .escaped_argfunction
.mapped_argfunction 1 2 3 {echo a $* a}
.mapped_argfunction
fn g .{argfunction} {.nop
     .whence_function .argfunction {.argfunction}
     .whence_function .argfunction {.escaped_argfunction}}
g {.whence_function .argfunction {}
     .whence_function .argfunction {.argfunction}
     .whence_function .argfunction {.escaped_argfunction}}
g {}

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
fn tribble [args ...] {
  .replace_exception tribble $args$
  .replace_exception tribble $args$
  echo the multiplication has begun $#}
.try_catch_recursive tribble {.throw tribble}
.throw tribble
fn neverending [args ...] {.replace_exception neverending $args$}
.throw neverending
.try_catch_recursive neverending {.throw neverending}
fn sneaky [args ...] {
  .try_catch_recursive sneaky {.replace_exception sneaky $args$}}
.try_catch_recursive sneaky {.throw sneaky}

# .run_logic
fn .run_logic flag cmd ... {
  .if .test_number_equal 0 $flag {.nop}
  .else_if $cmd$ {.nop}
  .else {.nop}}
0 echo don't print nuthin'
1 echo do print
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
single * 2 - 40 10
single - 40 - 10 2
single / - + 40 10 2 2
single - 40 - * 2 5 - 7 2
.function excessive-commentary arg {#! shebang
  $arg first comment before a blank line

  echo not a $arg; .nop second; echo either}
excessive-commentary #
echo-comments excessive-commentary #

# ln
ln -s test_files/fibb
readlink ixxx
ln -s test_files/ixxx
readlink ixxx
/bin/rm ixxx
ln -s -t test_files/ rwshlib.h plumber.h builtin.h
readlink test_files/rwshlib.h test_files/plumber.h test_files/builtin.h
ln -s -t test_files/ ../rwshlib.h ../plumber.h ../builtin.h
readlink test_files/rwshlib.h test_files/plumber.h test_files/builtin.h
/bin/rm test_files/rwshlib.h test_files/plumber.h test_files/builtin.h

## environment testing i don't want to mess up everything else
# check for extraneous variables and that export_env doesn't bless
.scope M val (N necessary) {.nop $necessary
  printenv -- .var_exists -* ? A C PATH SHELL TESTABILITY args N necessary nl}

# .list_environment
.set SHELL unmodified
.list_environment x
.list_environment {excess argfunc}
.for ${.list_environment}$ (-- var val) {
  .if .test_in $var ? FIGNORE SHELL {echo $var : $$var to $val}
  .else {.global $var $val}}
.combine $TESTABILITY $nl
echo $SHELL
.unset TESTABILITY
.for ${.list_environment}$ (-- j ...) {
  .scope $j$ (-- var val) {setf $var $val; .nop $$var}}
.combine $TESTABILITY $nl
echo $SHELL
printenv -- -* .var_exists ? A C PATH SHELL TESTABILITY args argv broken nl
whence printenv

if_only_not test -z z {echo test throwing a .false exception}

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
.scope ([-a a1 a2 a3] [-b] [-c] [-d d1]) {.list_locals; .echo $nl}
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
.scope x y z (x y z) {.reinterpret $x (x [y] z)}
.scope w x (w x) {.local y y; .reinterpret $w $x (w [x y])}

# .excessive_nesting Base_executable::exception_handler
fn g {h}
fn h {g}
g
.stepwise g (argv ...) {echoe $argv$ $nl; $*}
fn .excessive_nesting args ... {.nop $args; h}
g
fn .excessive_nesting args ... {.nop $args; echoe &&{.throw .nop}}
fn .failed_substitution args ... {.nop $args; echoe $Z}
g
.set_max_extra_exceptions 0
e_after {.try_catch_recursive .undeclared_variable .excessive_nesting .failed_substitution {g}}
.set_max_extra_exceptions 5
e_after {.try_catch_recursive .undeclared_variable .excessive_nesting .failed_substitution {g}}
e_after {.try_catch_recursive .undeclared_variable .failed_substitution {
  e ${.throw .nop}}}
fn .else_without_if args ... {.nop $args; e ${.throw .nop}}
e_after {.try_catch_recursive .undeclared_variable .else_without_if {
  .else {}}}
fn .failed_substitution args ... {.nop $args; e ${.throw echo foo}}
e_after {.try_catch_recursive .undeclared_variable .failed_substitution {
  e ${.throw .nop}}}
fn .else_without_if args ... {.nop $args; e $Z}
e_after {.try_catch_recursive .undeclared_variable .else_without_if {
  .else {}}}
fn .failed_substitution args ... {.nop $args; .test_less 0 Z}
e_after {.try_catch_recursive .not_a_number .failed_substitution {
  e ${.throw .nop}}}

# .getpid .getppid .sighup
.getpid excess
.getpid {excess argfunc}
.getppid excess
.getppid {excess argfunc}
se {.fork se {
      /bin/kill -HUP ${.getppid}
      echo after the signal in subshell}
    echo after the signal in parent}
se {.fork se {
      /bin/kill -HUP ${.getpid}
      echo after the signal in subshell}
    echo after the signal in parent}
.while .nop {
  echo before signals
  /bin/kill -PIPE ${.getpid}
  echo after sigpipe
  /bin/kill ${.getpid}
  echo should not continue beyond SIGTERM}
.while .nop {
  echo before signals
  /bin/kill -USR1 ${.getpid}
  echo after sigusr1
  /bin/kill -INT ${.getpid}
  echo should not continue beyond SIGINT}

# exiting
# .shutdown .exit
.exit
.exit 1 {excess argfunction}
.exit not_a_number
.fork .exit 256
.fork .exit -2560
.fork .exit -2
fn exit_exception args ... {echo $args$; .exit 0}
.collect_errors_except .nop {
  .try_catch_recursive exit_exception {
    .collect_errors_only exit_exception {
      .throw exit_exception
      echo print expected after having collected exit exception}}
  echo after having run .exit (will not print)}
.echo 1 2 3
