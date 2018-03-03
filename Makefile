CFLAGS  += -Wall -Wextra
LDLIBS   = -lcurl -ljson-c

all: nostt

nostt: nostt.o api.o
	$(CC) $(LDFLAGS) -o $@ nostt.o api.o $(LDLIBS)

clean:
	rm -f *.o nostt

nostt.o: api.h
api.o:   api.h

.PHONY: all clean
