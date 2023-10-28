.PHONY=all clean

all:
	if [ ! -d "bin" ]; then mkdir bin; fi
	clang src/machine.c src/handlers.c src/memory.c src/objreader.c -Wall -g -o bin/machine
	clang src/asm.c -Wall -g -o bin/asm

clean:
	rm -rf bin/*