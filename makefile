# makefile for rwsh

objects = argm.o arg_script.o arg_spec.o builtin.o clock.o command_stream.o \
	default_stream.o executable.o executable_map.o file_stream.o \
	function.o plumber.o rwsh.o rwsh_stream.o rwshlib.o pipe_stream.o \
	prototype.o selection.o substitution_stream.o variable_map.o
local_objects = rwsh_init.o 
	
#CC = g++-7
#CXX = g++-7
CC = g++
CXX = g++
CXXFLAGS += -g -std=c++11
#CXXFLAGS += -g -std=c++17
LDLIBS = -lreadline

rwsh: $(objects) $(local_objects)
librwsh.a: $(objects)
	ar -ruv librwsh.a $(objects)

arg_script.o: arg_spec.h rwsh_stream.h argm.h arg_script.h executable.h \
	file_stream.h function.h prototype.h variable_map.h
arg_spec.o: arg_spec.h rwsh_stream.h argm.h argm_star_var.cc arg_script.h \
	executable.h executable_map.h function.h pipe_stream.h prototype.h \
	read_dir.cc selection.h selection_read.cc substitution_stream.h \
	tokenize.cc prototype.h variable_map.h
argm.o: arg_spec.h rwsh_stream.h argm.h arg_script.h executable.h function.h \
	prototype.h variable_map.h
builtin.o: arg_spec.h rwsh_stream.h argm.h argm_star_var.cc arg_script.h \
	builtin.h clock.h command_stream.h executable.h executable_map.h \
	file_stream.h function.h plumber.cc pipe_stream.h prototype.h read_dir.cc \
	selection.h substitution_stream.h tokenize.cc variable_map.h
clock.o: clock.h
command_stream.o: arg_spec.h rwsh_stream.h argm.h arg_script.h clock.h \
	command_stream.h executable.h executable_map.h function.h prototype.h \
	variable_map.h
default_stream.o: rwsh_stream.h default_stream.h
executable.o: rwsh_stream.h argm.h builtin.h clock.o executable.h \
	executable_map.h plumber.h variable_map.h
executable_map.o: arg_spec.h rwsh_stream.h argm.h arg_script.h executable.h \
	executable_map.h function.h prototype.h
file_stream.o: rwsh_stream.h argm.h file_stream.h variable_map.h
function.o: arg_spec.h rwsh_stream.h argm.h arg_script.h clock.h executable.h \
	executable_map.h function.h prototype.h variable_map.h
plumber.o: rwsh_stream.h argm.h clock.h executable.h plumber.h variable_map.h
prototype.o: arg_spec.h rwsh_stream.h argm.h arg_script.h executable.h \
	prototype.h variable_map.h
rwsh.o: arg_spec.h rwsh_stream.h argm.h argm_star_var.cc arg_script.h \
	clock.o command_stream.h default_stream.h executable.h \
	executable_map.h function.h plumber.h prototype.h rwsh_init.h \
	selection.h variable_map.h
rwsh_init.o: arg_spec.h rwsh_stream.h arg_script.h argm.h rwsh_init.h \
	builtin.h executable.h executable_map.h function.h prototype.h
rwsh_stream.o: rwsh_stream.h
rwshlib.o: rwshlib.h
selection.o: rwsh_stream.h argm.h selection.h tokenize.cc variable_map.h
substitution_stream.o: argm.h rwsh_stream.h pipe_stream.h plumber.h \
    substitution_stream.h variable_map.h
pipe_stream.o: argm.h rwsh_stream.h variable_map.h
variable_map.o: arg_spec.h rwsh_stream.h argm.h arg_script.h executable.h \
	variable_map.h

.PHONY: all clean dist
all: rwsh librwsh.a
clean:
	rm *.o
dist:
	mkdir rwsh-0.3
	cp -r *.h *.cc *.sh rwshrc* \
		COPYING Changelog docs KnownBugs makefile README test.result \
		rwsh-0.3
	tar -czf rwsh-0.3.tar.gz rwsh-0.3
	rm -r rwsh-0.3
install: rwsh
	install rwshrc-basic /etc/
	install rwshrc-default /etc/
	install rwshrc /etc/
	install rwsh /bin
cowboy-install: rwsh
	ln -sf $(CURDIR)/rwshrc-basic /etc/
	ln -sf $(CURDIR)/rwshrc-default /etc/
	ln -sf $(CURDIR)/rwshrc /etc/
	ln -sf $(CURDIR)/rwsh /bin

