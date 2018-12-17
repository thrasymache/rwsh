.source /etc/rwshrc-basic
.global PATH /bin:/usr/bin
.scope () {.getpid; .echo $nl}
.scope () {.getppid; .echo $nl}
.function_all_flags timings focus {
  .combine $focus (:	last\() ${.last_execution_time $focus} \) \	
  .combine (total\() ${.total_execution_time $focus} \) \	
  .combine (count\() ${.execution_count $focus} \) $nl}
.function_all_flags clock-watch {
  .usleep 600
  /bin/true
  timings timings
  timings clock-watch
  .combine (usleep overhead:	) ${.usleep_overhead} $nl
  timings .usleep
  timings /bin/true}
clock-watch
clock-watch
.mapped_argfunction {timings timings; timings clock-watch}
.error a stderr print will come before the diff $nl
.nop $A $FIGNORE

