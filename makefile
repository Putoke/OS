all:
	gcc -pedantic -Wall -ansi -O4 main.c util.c syscalls.c

clean:
	rm *.out

run:
	./a.out