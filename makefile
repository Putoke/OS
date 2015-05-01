all:
	gcc -pedantic -Wall -ansi -O4 *.c

clean:
	rm *.out

run:
	./a.out