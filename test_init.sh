%set MAX_NESTING 4
%function rwsh.raw_command {%echo $1; %newline}
%function rwsh.after_command {%echo $?; %newline; %if_errno {%echo ERRNO set to $ERRNO; %newline; %set ERRNO \ }}
%function rwsh.prompt {%echo \$}
%function rwsh.vars {%internal_vars}
%function # {%nop}
%function \ {%nop}
