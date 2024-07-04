# makefile for rwsh

objects = rwsh.o argm.o arg_script.o arg_spec.o builtin.o call_stack.o clock.o \
	command_stream.o default_stream.o executable.o executable_map.o \
	file_stream.o function.o plumber.o rwsh_stream.o rwshlib.o \
	pipe_stream.o prototype.o selection.o substitution_stream.o \
	variable_map.o
local_objects = rwsh_init.o 
documentation = docs/argument_functions.html docs/builtins.html \
	docs/function_prototypes_and_option_handling.html \
	docs/internal_functions.html docs/introduction_and_philosophy.html \
	docs/the_selection.html
	
CXX = c++
#CXX = g++
CXXFLAGS += -g -std=c++17 -I /usr/local/include
LDFLAGS += -L /usr/local/lib
LDLIBS = -lreadline

rwsh: $(objects) $(local_objects)
	$(CXX) $(LDFLAGS) $(objects) $(local_objects) $(LDLIBS) -o $@
librwsh.a: $(objects)
	ar -rv $@ $(objects)
%.html: %.md
	markdown $< >$@
#deps.mk: $(objects:o=cc) $(local_objects:o=cc)
#	gcc >$@ -MM $^


include deps.mk

.PHONY: all clean dist test
all: rwsh librwsh.a deps.mk
clean:
	rm *.o
dist:
	mkdir rwsh-0.3
	cp -r *.h *.cc *.sh rwshrc* \
		COPYING Changelog docs KnownBugs makefile README test.result \
		rwsh-0.3
	tar -czf rwsh-0.3.tar.gz rwsh-0.3
	rm -r rwsh-0.3
test:
	./rwsh -to <test_main.sh
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
docs: $(documentation)
docs-clean:
	rm $(documentation)
