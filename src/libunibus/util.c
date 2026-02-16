#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

int
setup_socket_dir()
{
	int ret, err;
	struct stat dir_stat;
	ret = stat(SOCK_DIR, &dir_stat);
	if (ret == 0) {
		return 0;
	}

	err = errno;
	if (err != ENOENT) {
		perror("setup_socket_dir-stat");
		return -1;
	}

	ret = mkdir(SOCK_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	if (ret != 0) {
		perror("setup_socket_dir-mkdir");
		return -1;
	}

	return 0;
}
