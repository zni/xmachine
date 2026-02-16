#ifndef LIBUNIBUS_UTIL_H
#define LIBUNIBUS_UTIL_H

#define SOCK_DIR "/tmp/xmachine"

/* Direction to send a message on the bus. */
/* XXX Why did I even add in directionality? */
typedef enum _direction {
	D_LEFT,
	D_RIGHT,
	D_NONE
} direction;

#define DIRC(x) (x == D_LEFT ? "L" : (x == D_RIGHT ? "R" : "NONE"))


int setup_socket_dir();

#endif
