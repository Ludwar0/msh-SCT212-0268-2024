CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2

.PHONY: all test clean
all: msh

msh: msh.c
	$(CC) $(CFLAGS) -o $@ $<

test: msh
	./tests/test_msh.sh

clean:
	rm -f msh

# The test target runs the non-interactive smoke suite.
