CC		= g++
CFLAGS	= -Wall -pipe -O2 -ggdb

BINS	:= bicat
OBJS	:= bicat.o ip.o

all:	$(BINS)

bicat:	$(OBJS)
	$(CC) -o $@ $<

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(BINS)
