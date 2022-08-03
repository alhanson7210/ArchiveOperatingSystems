bin=flash

# Set the following to '0' to disable log messages:
debug=0

CFLAGS += -Wall -g -DDEBUG=$(debug)
LDFLAGS +=

src=shell.c history.c prompt.c string_functions.c signal_handling.c built_in_commands.c
obj=$(src:.c=.o)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

shell.o: shell.c history.h prompt.h signal_handling.h string_functions.h built_in_commands.h debug.h
history.o: history.c history.h string_functions.h built_in_commands.h debug.h
prompt.o: prompt.c prompt.h string_functions.h debug.h
signal_handling.o: signal_handling.h signal_handling.c debug.h
string_functions.o: string_functions.c string_functions.h debug.h
built_in_commands.o: built_in_commands.c built_in_commands.h string_functions.h history.h prompt.h debug.h

clean:
	rm -f $(bin) $(obj)


# Tests --

test: $(bin) ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	git submodule update --init --remote

testclean:
	rm -rf tests
