.PHONY=all clean

all:
	if [ ! -d "bin" ]; then mkdir bin; fi
	clang src/machine.c src/handlers.c src/tty.c -Wall -g -o bin/machine
	clang src/asm.c -Wall -g -o bin/asm

clean:
	rm -rf bin/*