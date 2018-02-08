CFLAGS  += -Wall -I/usr/local/include -O2
LDFLAGS += -L/usr/local/lib
LDLIBS   = -lcurl -lyajl

all: nostt

clean: ; rm -f nostt
