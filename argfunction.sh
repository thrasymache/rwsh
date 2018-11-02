.echo $nl $nl $nl $nl
.return 0
.function_all_flags # [args ...] {.nop $args$}
.function_all_flags fw {
  .function_all_flags f1 name {rwsh.argfunction}
  .whence_function f1; .echo $nl
  f1 f2 {rwsh.argfunction}
  .whence_function f2; .echo $nl
  .rm_executable f1; .rm_executable f2}
fw {.echo hi $nl; .function_all_flags $name {rwsh.argfunction}}
fw {.while .return 1 {.function_all_flags &&name {rwsh.argfunction}}}
.function_all_flags f1 name {
  .echo function name $name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}
f1 f2 {
  .echo noname $1 $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of nothing $nl}
fw {
  .echo function name $name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}
fw {.for 1 2 3 {
  .echo function name $1 $name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
fw {.for 1 2 3 {rwsh.mapped_argfunction $1 {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
# all of the above make sense, and the next should be identical to the last
fw {rwsh.mapped_argfunction $1 {.for 1 2 3 {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
# mostly as wierd
fw {rwsh.mapped_argfunction rwsh.not_a_number {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
# normal again
fw {.try_catch_recursive rwsh.not_a_number {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
fw {.for 1 2 3 {.try_catch_recursive rwsh.not_a_number {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
fw {.try_catch_recursive rwsh.not_a_number {.for 1 2 3 {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
# maybe weird?
fw {rwsh.mapped_argfunction $1 {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
# definitely weird
fw {rwsh.mapped_argfunction $name {.for a b {
  .echo function name &&name $nl
  .whence_function rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
fw {rwsh.mapped_argfunction {.function_all_flags $name {rwsh.argfunction}}}
.return 0
.nop $A $FIGNORE
.function_all_flags rwsh.shutdown -- args ... {
  .nop $args
  .combine $nl (now terminating normally) $nl}
.exit

