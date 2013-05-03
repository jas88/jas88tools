CC		= gcc
CFLAGS	= -Wall -pipe -O2 -ggdb

BINS	:= bicat
OBJS	:= bicat.o

all:	$(BINS)

bicat:	$(OBJS)
	$(CC) -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(BINS)
