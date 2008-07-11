# makefile for rwsh

objects = argv.o arg_script.o arg_spec.o builtin.o clock.o command_stream.o \
	default_stream.o executable.o executable_map.o file_stream.o \
	function.o plumber.o rwsh.o rwsh_stream.o selection.o \
	substitution_stream.o variable_map.o
	
CXXFLAGS = -g
CC = g++

rwsh: $(objects)

arg_script.o: arg_spec.h rwsh_stream.h argv.h arg_script.h executable.h \
	file_stream.h function.h variable_map.h
arg_spec.o: arg_spec.h rwsh_stream.h argv.h argv_star_var.cc arg_script.h \
	executable.h executable_map.h function.h read_dir.cc selection.h \
	selection_read.cc substitution_stream.h tokenize.cc variable_map.h
argv.o: arg_spec.h rwsh_stream.h argv.h arg_script.h executable.h function.h \
	variable_map.h
builtin.o: arg_spec.h rwsh_stream.h argv.h arg_script.h builtin.h clock.h \
	command_stream.h executable.h executable_map.h function.h \
	plumber.cc read_dir.cc selection.h tokenize.cc variable_map.h
clock.o: clock.h
command_stream.o: arg_spec.h rwsh_stream.h argv.h arg_script.h clock.h \
	command_stream.h executable.h executable_map.h variable_map.h
default_stream.o: rwsh_stream.h default_stream.h
executable.o: rwsh_stream.h argv.h builtin.h clock.o executable.h \
	executable_map.h plumber.h variable_map.h
executable_map.o: arg_spec.h rwsh_stream.h argv.h arg_script.h executable.h \
	executable_map.h function.h
file_stream.o: rwsh_stream.h argv.h file_stream.h
function.o: arg_spec.h rwsh_stream.h argv.h arg_script.h clock.h executable.h \
	executable_map.h function.h tokenize.cc variable_map.h
plumber.o: rwsh_stream.h argv.h clock.h executable.h plumber.h
rwsh.o: arg_spec.h rwsh_stream.h argv.h arg_script.h builtin.h clock.o \
	command_stream.h default_stream.h executable.h executable_map.h \
	function.h plumber.h selection.h variable_map.h
rwsh_stream.o: rwsh_stream.h
selection.o: rwsh_stream.h argv.h selection.h tokenize.cc
substitution_stream.o: rwsh_stream.h plumber.h substitution_stream.h
variable_map.o: arg_spec.h rwsh_stream.h argv.h arg_script.h executable.h \
	variable_map.h

.PHONY: all clean dist
all: rwsh
clean:
	rm *.o
dist:
	tar -czf rwsh-0.1.tar.gz *.h *.cc COPYING ChangeLog makefile README \
		test_init.sh test_main.sh test.result test_rwsh

