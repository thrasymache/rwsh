%set MAX_NESTING 5
%function rwsh.raw_command {%echo $1; %newline}
%function # {%nop}
%function \ {%nop}
%global last_command_return -10
%global A \
%global N \
%function rwsh.before_command {%return $last_command_return}

# tests multi-line commands within a sourced script
%function rwsh.after_command {
  %set last_command_return $?
  %newline
  %echo $last_command_return
  %newline
  %if_errno {
    %echo ERRNO set to $ERRNO
    %newline
    %unset ERRNO}}
%function rwsh.prompt {%echo \$}
%function rwsh.shutdown {e rwsh is now terminating}
%function rwsh.vars {%internal_vars}
