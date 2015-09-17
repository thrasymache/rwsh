.which_executable rwsh.argfunction {
  .function  x  { &&{ .which_path  echo  $PATH }  something } }
.which_executable rwsh.argfunction {{{{{{{{{{{}  }} }}}}}}}}
.function w { .which_executable  $1  {rwsh.argfunction} }
.which_executable w
w w
