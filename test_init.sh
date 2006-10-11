%set MAX_NESTING 4
%set last_command_return -10
%function rwsh.raw_command {%echo $1; %newline}
%function rwsh.before_command {%return $last_command_return}
%function rwsh.after_command {%set last_command_return $?; %echo $last_command_return; %newline; %if_errno {%echo ERRNO set to $ERRNO; %newline; %set ERRNO \ }}
%function rwsh.prompt {%echo \$}
%function rwsh.vars {%internal_vars}
%function # {%nop}
%function \ {%nop}
