include config.mk

TARGET  = nostt
OBJECTS = nostt.o api.o

all: nostt

nostt: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

nostt.o: api.h config.h
api.o:   api.h config.h

clean:
	rm -f *.o nostt
