CFLAGS += -std=c99 -g
CFLAGS += -Wall -Wextra -pedantic
CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += -I/usr/local/include

LDLIBS += -L/usr/local/lib -lcurl -ljson-c

# BSD conventions by default, override to taste
prefix  ?= /usr/local
bindir  ?= $(prefix)/bin
man1dir ?= $(prefix)/man/man1

all: nostt

clean:
	rm -f *.o nostt

install: nostt
	install nostt $(bindir)/
	install nostt.1 $(man1dir)/

uninstall:
	rm -f $(bindir)/nostt
	rm -f $(man1dir)/nostt.1

nostt: nostt.o api.o compat.o
	$(CC) $(LDFLAGS) -o $@ nostt.o api.o compat.o $(LDLIBS)

nostt.o:  api.h compat.h
api.o:    api.h
compat.o: compat.h

.PHONY: all clean
