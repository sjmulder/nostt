DESTDIR   ?=
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)/man

CFLAGS += -Wall -Wextra
LDLIBS += -lcurl -ljson-c

all: nostt

check: nostt
	./nostt 100 >/dev/null

clean:
	rm -f *.o nostt

install: nostt
	install -d $(DESTDIR)$(PREFIX)/bin \
	           $(DESTDIR)$(MANPREFIX)/man1 \
	           $(DESTDIR)$(MANPREFIX)/nl/man1
	install -m755 nostt      $(DESTDIR)$(PREFIX)/bin/
	install -m644 nostt.1    $(DESTDIR)$(MANPREFIX)/man1/
	install -m644 nostt.1.nl $(DESTDIR)$(MANPREFIX)/nl/man1/nostt.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/nostt
	rm -f $(DESTDIR)$(MANPREFIX)/man1/nostt.1
	rm -f $(DESTDIR)$(MANPREFIX)/man1/nl/nostt.1

nostt: nostt.o api.o compat.o
	$(CC) $(LDFLAGS) -o $@ nostt.o api.o compat.o $(LDLIBS)

nostt.o:  api.h compat.h
api.o:    api.h
compat.o: compat.h

.PHONY: all check clean install uninstall
