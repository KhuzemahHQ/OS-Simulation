# makefile

all: memtest

ackermann.o: ackermann.c ackermann.h 
	g++ -c -g ackermann.c

my_allocator.o : my_allocator.c my_allocator.h
	g++ -c -g my_allocator.c

memtest: memtest.c ackermann.o my_allocator.o
	g++ -o memtest memtest.c ackermann.o my_allocator.o

