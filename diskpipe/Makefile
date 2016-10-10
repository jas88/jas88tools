TARGETS	:= diskpipe
CFLAGS	:= -Wall -Wextra -O2


.PHONY:	all clean


all:	diskpipe

diskpipe:	diskpipe.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TARGETS)