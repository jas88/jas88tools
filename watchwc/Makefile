TARGETS	:= watchwc

CC		:= gcc
CFLAGS	:= -Wall -O2

.PHONY:	all clean

all:	$(TARGETS)

watchwc:	watchwc.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	$(RM) $(TARGETS)