CFLAGS  += -Wall -Wextra
LDFLAGS +=
LDLIBS   = -lcurl -ljson-c

TARGET   = nostt
OBJECTS  = nostt.o api.o
PREFIX	?= /usr/local

all: nostt

nostt: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

clean:
	rm -f *.o nostt

install: nostt
	install -d $(PREFIX)/bin \
	           $(PREFIX)/share/man/man1 \
	           $(PREFIX)/share/doc/nostt
	install nostt $(PREFIX)/bin/
	install nostt.1 $(PREFIX)/share/man/man1/
	install LICENSE.md $(PREFIX)/share/doc/nostt/

uninstall:
	rm -f $(PREFIX)/bin/nostt \
	      $(PREFIX)/share/man/man1/nostt.1 \
	      $(PREFIX)/share/doc/nostt/LICENSE.md
	-rmdir $(PREFIX)/share/doc/nostt/

nostt.o: api.h
api.o:   api.h

.PHONY: all clean install uninstall
