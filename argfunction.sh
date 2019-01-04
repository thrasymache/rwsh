.echo $nl $nl $nl $nl
.return 0
.function_all_flags # [args ...] {.nop $args$}
.function_all_flags fw {
  .function_all_flags f1 name {.argfunction}
  .whence_function f1; .echo $nl
  f1 f2 {.argfunction}
  .whence_function f2; .echo $nl
  .rm_executable f1; .rm_executable f2}
fw {.echo hi $nl; .function_all_flags $name {.argfunction}}
fw {.while .throw .false {.function_all_flags &&name {.argfunction}}}
.function_all_flags f1 name {
  .echo function name $name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}
f1 f2 {
  .echo noname $1 $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of nothing $nl}
fw {
  .echo function name $name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}
fw {.for 1 2 3 {
  .echo function name $1 $name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}
fw {.for 1 2 3 {.mapped_argfunction $1 {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
# all of the above make sense, and the next should be identical to the last
fw {.mapped_argfunction $1 {.for 1 2 3 {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
# mostly as wierd
fw {.mapped_argfunction .not_a_number {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}
# normal again
fw {.try_catch_recursive .not_a_number {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}
fw {.for 1 2 3 {.try_catch_recursive .not_a_number {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
fw {.try_catch_recursive .not_a_number {.for 1 2 3 {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
# maybe weird?
fw {.mapped_argfunction $1 {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}
# definitely weird
fw {.mapped_argfunction $name {.for a b {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
fw {.mapped_argfunction {.function_all_flags $name {.argfunction}}}
.return 0
.nop $A $FIGNORE
.function_all_flags .shutdown -- args ... {
  .nop $args
  .combine $nl (now terminating normally) $nl}
.exit 0

