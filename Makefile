CPPFLAGS = -D_XOPEN_SOURCE=700L
CFLAGS   = -g -std=c99 -pedantic -Wall -Wno-deprecated-declarations
CC = cc

DEVICE_LDFLAGS = -lpthread
UNIBUS_LDFLAGS = -Lsrc/libunibus/ -lunibus
LOAD_LDFLAGS = -Lsrc/libload/ -lload

CPU_C = $(wildcard src/cpu/*.c)
CPU_OBJ = ${CPU_C:.c=.o}

MEM_C = $(wildcard src/mem/*.c)
MEM_H = $(wildcard src/mem/*.h)
MEM_OBJ = ${MEM_C:.c=.o}

LIBLOAD_C = $(wildcard src/libload/*.c)
LIBLOAD_H = $(wildcard src/libload/*.h)
LIBLOAD_OBJ = ${LIBLOAD_C:.c=.o}

LIBUNIBUS_C = $(wildcard src/libunibus/*.c)
LIBUNIBUS_H = $(wildcard src/libunibus/*.h)
LIBUNIBUS_OBJ = ${LIBUNIBUS_C:.c=.o}

all: cpu mem

${CPU_OBJ}: ${CPU_C}

cpu: libunibus ${CPU_OBJ}
	${CC} -o src/cpu/$@ ${CPU_OBJ} ${UNIBUS_LDFLAGS} ${DEVICE_LDFLAGS}

${MEM_OBJ}: ${MEM_C} ${MEM_H}

mem: libunibus libload ${MEM_OBJ}
	${CC} -o src/mem/$@ ${MEM_OBJ} ${DEVICE_LDFLAGS} ${UNIBUS_LDFLAGS} ${LOAD_LDFLAGS}

${LIBLOAD_OBJ}: ${LIBLOAD_C} ${LIBLOAD_H}

libload: ${LIBLOAD_OBJ}
	ar rcs src/libload/libload.a ${LIBLOAD_OBJ}

clean_libload:
	rm -f ${LIBLOAD_OBJ}
	rm -f src/libload/libload.a

${LIBUNIBUS_OBJ}: ${LIBUNIBUS_C} ${LIBUNIBUS_H}

libunibus: ${LIBUNIBUS_OBJ}
	ar rcs src/libunibus/libunibus.a ${LIBUNIBUS_OBJ}

clean_libunibus:
	rm -f ${LIBUNIBUS_OBJ}
	rm -f src/libunibus/libunibus.a

loader: libload src/utilities/loader.c
	${CC} src/utilities/loader.c ${LOAD_LDFLAGS} -o src/utilities/$@

clean: clean_libload clean_libunibus
	rm -rf bin
	rm -f src/cpu/cpu
	rm -f src/cpu/cpu.o
	rm -f src/mem/mem
	rm -f src/mem/mem.o
	rm -f src/utilities/loader


.PHONY: clean_libunibus clean_libload
