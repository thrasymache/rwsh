rwsh.o: rwsh.cc argv.h rwsh_stream.h call_stack.h clock.h \
 command_stream.h default_stream.h prototype.h rwsh_init.h selection.h \
 variable_map.h argm.h arg_script.h executable.h executable_map.h \
 plumber.h function.h
argm.o: argm.cc argv.h rwsh_stream.h call_stack.h prototype.h rwshlib.h \
 variable_map.h argm.h arg_script.h executable.h function.h
argv.o: argv.cc argv.h
arg_script.o: arg_script.cc argv.h rwsh_stream.h arg_spec.h call_stack.h \
 file_stream.h prototype.h variable_map.h argm.h arg_script.h \
 executable.h function.h
arg_spec.o: arg_spec.cc argv.h arg_spec.h rwsh_stream.h call_stack.h \
 pipe_stream.h prototype.h read_dir.cc rwshlib.h selection.h \
 substitution_stream.h tokenize.cc variable_map.h argm.h argm_star_var.cc \
 arg_script.h executable.h executable_map.h selection_read.cc function.h
builtin.o: builtin.cc argv.h rwsh_stream.h builtin.h call_stack.h clock.h \
 command_stream.h file_stream.h pipe_stream.h prototype.h read_dir.cc \
 rwshlib.h substitution_stream.h tokenize.cc variable_map.h argm.h \
 arg_script.h executable.h executable_map.h plumber.h selection.h \
 function.h
call_stack.o: call_stack.cc argv.h arg_spec.h rwsh_stream.h builtin.h \
 call_stack.h prototype.h variable_map.h argm.h arg_script.h executable.h \
 executable_map.h function.h
clock.o: clock.cc clock.h
command_stream.o: command_stream.cc argv.h rwsh_stream.h call_stack.h \
 clock.h command_stream.h prototype.h argm.h arg_script.h executable.h \
 executable_map.h function.h
default_stream.o: default_stream.cc rwsh_stream.h default_stream.h
executable.o: executable.cc argv.h rwsh_stream.h call_stack.h clock.h \
 argm.h executable.h executable_map.h plumber.h
executable_map.o: executable_map.cc argv.h rwsh_stream.h call_stack.h \
 prototype.h tokenize.cc variable_map.h argm.h arg_script.h executable.h \
 executable_map.h function.h
file_stream.o: file_stream.cc argv.h rwsh_stream.h file_stream.h
function.o: function.cc argv.h rwsh_stream.h call_stack.h prototype.h \
 variable_map.h argm.h arg_script.h executable.h executable_map.h \
 function.h
plumber.o: plumber.cc argv.h rwsh_stream.h call_stack.h clock.h plumber.h
rwsh_stream.o: rwsh_stream.cc rwsh_stream.h
rwshlib.o: rwshlib.cc rwshlib.h
pipe_stream.o: pipe_stream.cc argv.h rwsh_stream.h pipe_stream.h
prototype.o: prototype.cc argv.h rwsh_stream.h prototype.h variable_map.h \
 argm.h
selection.o: selection.cc argv.h rwsh_stream.h selection.h tokenize.cc \
 variable_map.h
substitution_stream.o: substitution_stream.cc rwsh_stream.h plumber.h \
 pipe_stream.h substitution_stream.h
variable_map.o: variable_map.cc argv.h rwsh_stream.h prototype.h \
 variable_map.h argm.h
rwsh_init.o: rwsh_init.cc argv.h rwsh_stream.h builtin.h call_stack.h \
 prototype.h arg_script.h rwsh_init.h executable.h executable_map.h \
 function.h
