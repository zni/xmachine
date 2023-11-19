.PHONY: all clean

all:
	if [ ! -d "bin" ]; then mkdir bin; fi
	clang++ -g -std=c++20 -pthread -lncurses src/OBJ.cpp src/Bus.cpp src/CPU.cpp src/Memory.cpp src/DiskController.cpp src/TTY.cpp src/Machine.cpp src/Main.cpp -o bin/xmachine

clean:
	rm bin/*