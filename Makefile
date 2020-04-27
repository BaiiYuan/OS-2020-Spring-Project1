all: main.o process.o
	gcc main.o process.o -o main
main.o: main.c process.o
	gcc -c main.c
process.o: process.c process.h
	gcc -c process.c
clean:
	rm -rf *o
	rm -f main
run:
	sudo ./main
