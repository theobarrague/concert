CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter -Wshadow \
         -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
         -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -O2 \
         -std=c11

all: concert

concert: *.c
	$(CC) $(CFLAGS) -o $@ $^

format:
	astyle --style=allman \
           --indent=spaces=4 \
           --indent-switches \
           --indent-labels \
           --pad-oper \
           --pad-comma \
           --pad-paren \
           --pad-header \
           --align-pointer=name \
           --align-reference=name \
           --convert-tabs \
           --attach-namespaces \
           --max-code-length=80 \
           --lineend=linux \
           --suffix=none \
           --break-blocks \
           *.c *.h

.PHONY: all format

