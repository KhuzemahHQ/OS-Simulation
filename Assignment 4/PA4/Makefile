GCC=/usr/bin/gcc

sasti_filesystem: shell.o fs.o disk.o
	$(GCC) shell.o fs.o disk.o -o sasti_filesystem

shell.o: shell.c
	$(GCC) -Wall shell.c -c -o shell.o -g

fs.o: fs.c fs.h
	$(GCC) -Wall fs.c -c -o fs.o -g

disk.o: disk.c disk.h
	$(GCC) -Wall disk.c -c -o disk.o -g

clean:
	rm sasti_filesystem disk.o fs.o shell.o

test:	$(SHELL_PROGRAM)
	@for test_script in tests/test_*.sh; do $${test_script}; done
