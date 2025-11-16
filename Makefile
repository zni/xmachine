.PHONY: tags bin clean_libunibus clean_libload

cpu: libunibus cpu.o bin
	gcc -o bin/cpu src/cpu/cpu.o -Llib/ -lunibus -lpthread

cpu.o: src/cpu/cpu.c src/cpu/cpu.h
	gcc -o src/cpu/cpu.o -c src/cpu/cpu.c

mem: libunibus libload mem.o bin
	gcc -o bin/mem src/mem/mem.o -Llib/ -lunibus -lload -lpthread

mem.o: src/mem/mem.c src/mem/mem.h
	gcc -c src/mem/mem.c -o src/mem/mem.o

disk: disk.o bin
	gcc -o bin/disk src/disk/disk.o

disk.o: src/disk/disk.c src/disk/disk.h
	gcc -c src/disk/disk.c -o src/disk/disk.o

tty: tty.o bin
	gcc -o bin/tty src/tty/tty.o

tty.o: src/tty/tty.c src/tty/tty.h
	gcc -c src/tty/tty.c -o src/tty/tty.o

bus: bus.o bin
	gcc -o bin/bus src/bus/bus.o

bus.o: src/bus/bus_arbitrator.c src/bus/bus_arbitrator.h
	gcc -c src/bus/bus_arbitrator.c -o src/bus/bus.o

libload: lib src/libload/load.c src/libload/load.h
	gcc -c src/libload/load.c -o src/libload/load.o
	ar rcs lib/libload.a src/libload/load.o

clean_libload:
	rm -f lib/libload.a
	rm -f src/libload/load.o

libunibus: lib src/libunibus/priority_bus.c src/libunibus/priority_bus.h src/libunibus/data_bus.c src/libunibus/data_bus.h src/libunibus/device_bus_mgr.c src/libunibus/device_bus_mgr.h
	(cd src/libunibus && gcc -c device_bus_mgr.c priority_bus.c data_bus.c && ar rcs ../../lib/libunibus.a device_bus_mgr.o data_bus.o priority_bus.o)

clean_libunibus:
	rm -f src/libunibus/device_bus_mgr.o
	rm -f src/libunibus/priority_bus.o
	rm -f src/libunibus/data_bus.o
	rm -f lib/libunibus.a

loader: libload src/utilities/loader.c bin
	gcc src/utilities/loader.c -L../src/libload -lload -o bin/loader

tags:
	ctags -R src

bin:
	mkdir -p bin

lib:
	mkdir -p lib

clean: clean_libload clean_libunibus
	rm -rf bin
	rm -f src/cpu/cpu.o
	rm -f src/disk/disk.o
	rm -f src/mem/mem.o
	rm -f src/tty/tty.o
	rm -f src/bus/bus.o

all: cpu bus mem


