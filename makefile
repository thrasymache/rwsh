# makefile for rwsh

objects = rwsh.o arg_script.o argv.o builtin.o command_stream.o \
		executable.o executable_map.o function.o rwsh_stream.o \
                selection.o util.o substitution_stream.o variable_map.o 
CXXFLAGS = -g
CC = g++

rwsh: $(objects)

arg_script.o: argv.h argv_star_var.cc arg_script.h executable.h \
	executable_map.h function.h read_dir.cc selection.h selection_read.cc \
	substitution_stream.h variable_map.h
argv.o: argv.h arg_script.h executable.h function.h rwsh_stream.h tokenize.cc \
	util.h variable_map.h
builtin.o: argv.h arg_script.h builtin.h executable.h executable_map.h \
	function.h read_dir.cc rwsh_stream.h selection.h tokenize.cc \
	variable_map.h
command_stream.o: argv.h arg_script.h command_stream.h executable.h \
	executable_map.h variable_map.h
executable.o: argv.h executable.h executable_map.h variable_map.h
executable_map.o: argv.h arg_script.h builtin.h executable.h executable_map.h \
	function.h
function.o: argv.h arg_script.h executable.h executable_map.h function.h \
	tokenize.cc
rwsh.o: argv.h arg_script.h command_stream.h executable.h executable_map.h \
	function.h variable_map.h
rwsh_stream.o: rwsh_stream.h
selection.o: argv.h read_dir.cc selection.h tokenize.cc util.h variable_map.h
substitution_stream.o: argv.h rwsh_stream.h substitution_stream.h
util.o: util.h
variable_map.o: argv.h variable_map.h

.PHONY: all clean dist
all: rwsh
clean:
	rm *.o
dist:
	tar -czf rwsh-0.1.tar.gz *.h *.cc COPYING ChangeLog makefile README \
		test.sh test.result test_rwsh

