#define _GNU_SOURCE
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s FILENAME\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	int mfd = memfd_create("snapshot", 0);
	if (mfd == -1) {
		perror("memfd_create");
		exit(EXIT_FAILURE);
	}

	// try to copy the damn thing (probably won't work)
	for (;;) {
		int ret = copy_file_range(fd, NULL, mfd, NULL, SIZE_MAX, 0);
		if (ret == -1) {
			perror("copy_file_range");
			exit(EXIT_FAILURE);
		}
		if (ret == 0) {
			break;
		}
		fprintf(stderr, "copied %d bytes\n", ret);
	}

	pause();
	exit(EXIT_SUCCESS);
}
