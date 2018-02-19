# Base
CFLAGS  = -ansi -Wall -O2
LDFLAGS =
LDLIBS  = -lcurl -ljson-c

# FreeBSD/clang
#CFLAGS  = -I/usr/local/include -ansi -Wall -O2 -Dinline=
#LDFLAGS = -L/usr/local/lib
#LDLIBS  = -lcurl -ljson-c
