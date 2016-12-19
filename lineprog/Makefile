all:	lineprog

lineprog:	lineprog.c stubs.o
	gcc -Wall -Wextra -s -static -nostdlib -o $@ $^

%.o:	%.s
	nasm -felf64 $^

clean:
	$(RM) lineprog
