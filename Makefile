CFLAGS  += -Wall -Wextra
LDFLAGS +=
LDLIBS   = -lcurl -ljson-c

OBJECTS  = nostt.o api.o
PREFIX	?= /usr/local

all: nostt

nostt: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

test: nostt
	./nostt -G 100 >/dev/null

clean:
	rm -f *.o nostt

nostt.o: api.h
api.o:   api.h

.PHONY: all clean install uninstall
