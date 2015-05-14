all:
	gcc -pedantic -Wall -ansi -O4 main.c

clean:
	rm *.out

run:
	./a.out