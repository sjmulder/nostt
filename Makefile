CFLAGS+=	-Wall -Wextra -g
LDLIBS+=	-lcurl -ljson-c

all: nostt

clean:
	rm -f *.o nostt

nostt: nostt.o api.o
	$(CC) $(LDFLAGS) -o $@ nostt.o api.o $(LDLIBS)

nostt.o: api.h
api.o:   api.h

.PHONY: all clean
