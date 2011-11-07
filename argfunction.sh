.echo $nl $nl $nl $nl
.return 0
.function fw {
  .function f1 {rwsh.argfunction}
  .which_executable f1; .echo $nl
  f1 f2 {rwsh.argfunction}
  .which_executable f2; .echo $nl
  .function f1; .function f2}
fw {.echo hi $nl; .function $1 {rwsh.argfunction}}
fw {.while .return 1 {.function $1 {rwsh.argfunction}}}
.function f1 {
  .echo function name $1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}
f1 f2 {
  .echo noname $1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of nothing $nl}
fw {
  .echo function name $1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}
fw {.for 1 2 3 {
  .echo function name $1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
fw {.for 1 2 3 {rwsh.mapped_argfunction $1 {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
fw {rwsh.mapped_argfunction $1 {.for 1 2 3 {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
fw {rwsh.mapped_argfunction rwsh.not_a_number {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
fw {.signal_handler rwsh.not_a_number {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
fw {.for 1 2 3 {.signal_handler rwsh.not_a_number {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
fw {.signal_handler rwsh.not_a_number {.for 1 2 3 {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
fw {rwsh.mapped_argfunction $1 {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}
fw {rwsh.mapped_argfunction $1 {.for a b {
  .echo function name &&1 $nl
  .which_executable rwsh.argfunction {rwsh.argfunction}
  .echo $nl end of function body $nl}}}
# fw {rwsh.mapped_argfunction {.function $1 {rwsh.argfunction}}}
.return 0
.exit

