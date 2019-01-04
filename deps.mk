argm.o: argm.cc arg_spec.h rwsh_stream.h rwshlib.h variable_map.h argm.h \
 arg_script.h call_stack.h executable.h prototype.h function.h
arg_script.o: arg_script.cc arg_spec.h rwsh_stream.h variable_map.h \
 argm.h arg_script.h call_stack.h executable.h file_stream.h prototype.h \
 function.h
arg_spec.o: arg_spec.cc arg_spec.h rwsh_stream.h variable_map.h argm.h \
 arg_script.h executable.h executable_map.h pipe_stream.h prototype.h \
 read_dir.cc rwshlib.h selection.h substitution_stream.h tokenize.cc \
 function.h argm_star_var.cc selection_read.cc
builtin.o: builtin.cc arg_spec.h rwsh_stream.h variable_map.h argm.h \
 arg_script.h builtin.h call_stack.h clock.h command_stream.h \
 executable.h executable_map.h file_stream.h pipe_stream.h plumber.h \
 prototype.h read_dir.cc rwshlib.h selection.h substitution_stream.h \
 tokenize.cc function.h
call_stack.o: call_stack.cc arg_spec.h rwsh_stream.h variable_map.h \
 argm.h arg_script.h builtin.h call_stack.h executable.h executable_map.h \
 prototype.h function.h
clock.o: clock.cc clock.h
command_stream.o: command_stream.cc arg_spec.h rwsh_stream.h \
 variable_map.h argm.h arg_script.h call_stack.h clock.h command_stream.h \
 executable.h executable_map.h prototype.h function.h
default_stream.o: default_stream.cc rwsh_stream.h default_stream.h
executable.o: executable.cc rwsh_stream.h variable_map.h argm.h \
 call_stack.h clock.h executable.h executable_map.h plumber.h
executable_map.o: executable_map.cc arg_spec.h rwsh_stream.h \
 variable_map.h argm.h arg_script.h call_stack.h executable.h \
 executable_map.h prototype.h tokenize.cc function.h
file_stream.o: file_stream.cc rwsh_stream.h variable_map.h argm.h \
 file_stream.h
function.o: function.cc arg_spec.h rwsh_stream.h variable_map.h argm.h \
 arg_script.h call_stack.h clock.h executable.h executable_map.h \
 prototype.h function.h
plumber.o: plumber.cc rwsh_stream.h variable_map.h argm.h call_stack.h \
 clock.h plumber.h
rwsh.o: rwsh.cc arg_spec.h rwsh_stream.h variable_map.h argm.h \
 arg_script.h call_stack.h clock.h command_stream.h default_stream.h \
 executable.h executable_map.h plumber.h prototype.h rwsh_init.h \
 selection.h function.h
rwsh_stream.o: rwsh_stream.cc rwsh_stream.h
rwshlib.o: rwshlib.cc rwshlib.h
pipe_stream.o: pipe_stream.cc rwsh_stream.h variable_map.h argm.h \
 pipe_stream.h
prototype.o: prototype.cc arg_spec.h rwsh_stream.h variable_map.h argm.h \
 arg_script.h executable.h prototype.h
selection.o: selection.cc rwsh_stream.h variable_map.h argm.h selection.h \
 tokenize.cc
substitution_stream.o: substitution_stream.cc rwsh_stream.h plumber.h \
 pipe_stream.h substitution_stream.h
variable_map.o: variable_map.cc rwsh_stream.h variable_map.h argm.h \
 executable.h
rwsh_init.o: rwsh_init.cc arg_spec.h rwsh_stream.h variable_map.h \
 arg_script.h argm.h rwsh_init.h builtin.h call_stack.h executable.h \
 executable_map.h prototype.h function.h
