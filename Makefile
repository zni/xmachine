.PHONY: clean tags bin

cpu: src/cpu/cpu.o bin
	gcc -o bin/cpu src/cpu/cpu.o

cpu.o: src/cpu/cpu.c src/cpu/cpu.h
	gcc -c src/cpu/cpu.c -o src/cpu/cpu.o

mem: src/mem/mem.o bin
	gcc -o bin/mem src/mem/mem.o

mem.o: src/mem/mem.c src/mem/mem.h
	gcc -c src/mem/mem.c -o src/mem/mem.o

disk: src/disk/disk.o bin
	gcc -o bin/disk src/disk/disk.o

disk.o: src/disk/disk.c src/disk/disk.h
	gcc -c src/disk/disk.c -o src/disk/disk.o

tty: src/tty/tty.o bin
	gcc -o bin/tty src/tty/tty.o

tty.o: src/tty/tty.c src/tty/tty.h
	gcc -c src/tty/tty.c -o src/tty/tty.o

bus: bus.o bin
	gcc -o bin/bus src/bus/bus.o

bus.o: src/bus/bus_arbitrator.c src/bus/bus_arbitrator.h
	gcc -c src/bus/bus_arbitrator.c -o src/bus/bus.o

libload: src/libload/load.c src/libload/load.h
	gcc -c src/libload/load.c -o src/libload/load.o
	ar rcs src/libload/libload.a src/libload/load.o

clean_libload: src/libload/libload.a
	rm src/libload/libload.a

libbus_pr: src/libbus/priority_bus.c src/libbus/priority_bus.h
	gcc -c src/libbus/priority_bus.c -o src/libbus/priority_bus.o

libbus_d: src/libbus/data_bus.c src/libbus/data_bus.h
	gcc -c src/libbus/data_bus.c -o src/libbus/data_bus.o

libbus: libbus_pr libbus_d
	ar rcs src/libbus/libbus.a src/libbus/data_bus.o src/libbus/priority_bus.o

clean_libbus: src/libbus/libbus.a
	rm -f src/libbus/priority_bus.o
	rm -f src/libbus/data_bus.o
	rm -f src/libbus/libbus.a

loader: libload src/utilities/loader.c bin
	gcc src/utilities/loader.c -L../src/libload -lload -o bin/loader

tags:
	ctags -R src

bin:
	mkdir -p bin

clean:
	rm -rf bin
	rm -f src/disk/disk.o
	rm -f src/mem/mem.o
	rm -f src/cpu/cpu.o
	rm -f src/tty/tty.o
	rm -f src/bus/bus.o

