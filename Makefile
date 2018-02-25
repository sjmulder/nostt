CFLAGS  += -ansi -Wall -Dinline=
LDFLAGS +=
LDLIBS   = -lcurl -ljson-c

TARGET  = nostt
OBJECTS = nostt.o api.o

all: nostt

nostt: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

nostt.o: api.h
api.o:   api.h

clean:
	rm -f *.o nostt
