.echo $nl $nl $nl $nl
.function fw .{argfunction} {
  .function f1 name .{argfunction} {.argfunction}
  .whence_function f1; .echo $nl
  f1 f2 {.argfunction}
  .whence_function f2; .echo $nl
  .rm_executable f1; .rm_executable f2}
fw {.echo hi $nl; .function $name .{argfunction} {.argfunction}}
fw {.while .throw .false {
  .function &&name .{argfunction} {.argfunction}}}
.function f1 name .{argfunction} {
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
fw {forj 1 2 3 {
  .echo function name $j $name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}
fw {forj 1 2 3 {.mapped_argfunction $j {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
# all of the above make sense, and the next should be identical to the last
fw {.mapped_argfunction $1 {forj 1 2 3 {.nop $j
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
fw {forj 1 2 3 {.try_catch_recursive .not_a_number {
  .nop $j
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
fw {.try_catch_recursive .not_a_number {forj 1 2 3 {.nop $j
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
# maybe weird?
fw {.mapped_argfunction $1 {
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}
# definitely weird
fw {.mapped_argfunction $name {forj a b {.nop $j
  .echo function name &&name $nl
  .whence_function .argfunction {.argfunction}
  .echo $nl end of function body $nl}}}
fw {.mapped_argfunction {
  .function $name .{argfunction} {.argfunction}}}
.nop $A $FIGNORE
.exit 0

