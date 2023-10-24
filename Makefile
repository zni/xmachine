.PHONY=all clean

all:
	if [ ! -d "bin" ]; then mkdir bin; fi
	clang src/machine.c src/handlers.c -g -o bin/machine
	clang src/asm.c -g -o bin/asm

clean:
	rm -rf bin/*