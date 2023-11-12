.PHONY=all clean

all:
	if [ ! -d "bin" ]; then mkdir bin; fi
	gcc -std=c11 -pthread src/machine.c src/handlers.c src/memory.c src/objreader.c src/tty.c src/disk.c -Wall -g -o bin/machine
	gcc src/asm.c -Wall -g -o bin/asm

clean:
	rm -rf bin/*