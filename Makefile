DESTDIR   ?=
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)/man

CFLAGS += -std=c99 -g
CFLAGS += -Wall -Wextra -pedantic
CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += -I/usr/local/include

LDLIBS += -L/usr/local/lib -lcurl -ljson-c

all: nostt

clean:
	rm -f *.o nostt

install: nostt
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(MANPREFIX)/man1
	install nostt   $(DESTDIR)$(PREFIX)/bin/
	install nostt.1 $(DESTDIR)$(MANPREFIX)/man1/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/nostt
	rm -f $(DESTDIR)$(MANPREFIX)/man1/nostt.1

nostt: nostt.o api.o compat.o
	$(CC) $(LDFLAGS) -o $@ nostt.o api.o compat.o $(LDLIBS)

nostt.o:  api.h compat.h
api.o:    api.h
compat.o: compat.h

.PHONY: all clean install uninstall
