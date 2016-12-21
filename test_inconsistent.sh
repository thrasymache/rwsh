.getpid
.getppid
.function_all_flags timings focus {
  .combine $focus (:	last\() ${.which_last_execution_time $focus} \) \	
  .combine (total\() ${.which_total_execution_time $focus} \) \	
  .combine (count\() ${.which_execution_count $focus} \) $nl}
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
rwsh.mapped_argfunction {timings timings; timings clock-watch}

