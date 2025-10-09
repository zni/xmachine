.PHONY: clean tags

cpu: src/cpu/cpu.o
	gcc -o bin/cpu src/cpu/cpu.o

cpu.o: src/cpu/cpu.c src/cpu/cpu.h
	gcc -c src/cpu/cpu.c -o src/cpu/cpu.o

mem: src/mem/mem.o
	gcc -o bin/mem src/mem/mem.o

mem.o: src/mem/mem.c src/mem/mem.h
	gcc -c src/mem/mem.c -o src/mem/mem.o

disk: src/disk/disk.o
	gcc -o bin/disk src/disk/disk.o

disk.o: src/disk/disk.c src/disk/disk.h
	gcc -c src/disk/disk.c -o src/disk/disk.o

tty: src/tty/tty.o
	gcc -o bin/tty src/tty/tty.o

tty.o: src/tty/tty.c src/tty/tty.h
	gcc -c src/tty/tty.c -o src/tty/tty.o

bus: src/bus/bus.o
	gcc -o bin/bus src/bus/bus.o

bus.o: src/bus/bus.c src/bus/bus.h
	gcc -c src/bus/bus.c -o src/bus/bus.o

libload: src/libload/load.c src/libload/load.h
	gcc -c src/libload/load.c -o src/libload/load.o
	ar rcs src/libload/libload.a src/libload/load.o

clean_libload: src/libload/libload.a
	rm src/libload/libload.a

loader: loader.c
	gcc loader.c -L../src/libload -lload -o bin/loader

clean_loader: bin/loader
	rm bin/loader

tags:
	ctags -R src

clean:
	rm bin/mem
	rm bin/cpu
	rm bin/disk
	rm bin/tty
	rm bin/bus
	rm src/disk/disk.o
	rm src/mem/mem.o
	rm src/cpu/cpu.o
	rm src/tty/tty.o
	rm src/bus/bus.o

