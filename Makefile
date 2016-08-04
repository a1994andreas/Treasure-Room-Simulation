#
# Makefile
# csd3031
# csd3031@csd.uoc.gr
# hy345
# ask2
#

# the compiler to use
CC=gcc

# the options i'll pass
OFLAGS=-o
CFLAGS=-c
PFLAGS=-pthread
# system dependent
	REM=rm -f
	XF=.out
	RUN=./


all: server client

server: server.o
	$(CC) $(OFLAGS) server$(XF) server.o $(PFLAGS)
	#$(RUN)assignment1$(XF)

server.o: server.c
	$(CC) $(CFLAGS) server.c

client: client.o
	$(CC) $(OFLAGS) client$(XF) client.o $(PFLAGS)

client.o: client.c
	$(CC) $(CFLAGS) client.c

clean:
	$(REM) *$(XF)
	$(REM) *.o
	$(REM) *.in
