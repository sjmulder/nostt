CFLAGS += -I/usr/local/include -Wall -Wextra -g
LDLIBS += -L/usr/local/lib -lcurl -ljson-c

all: nostt

clean:
	rm -f *.o nostt

nostt: nostt.o api.o compat.o
	$(CC) $(LDFLAGS) -o $@ nostt.o api.o compat.o $(LDLIBS)

nostt.o:  api.h compat.h
api.o:    api.h
compat.o: compat.h

.PHONY: all clean
