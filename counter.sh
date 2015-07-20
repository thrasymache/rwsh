.function rwsh.shutdown {/usr/bin/printenv}
.function rwsh.autofunction {.global var_name $1; .if .test_not_empty $$var_name {.var_add $var_name 1}; .else {.set $var_name 1}; .unset var_name}
.function rwsh.executable_not_found {.nop}
.unset rc
