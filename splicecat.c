#define _GNU_SOURCE
#include <fcntl.h>
#include <liburing.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#define QUEUE_DEPTH 1
#define BLOCK_SZ 1024

int main(int argc, char *argv[])
{
	struct io_uring ring;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s FILENAME [FILENAME ...]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Need one for each file, and then one for the pipe and splice */
	int depth = argc - 1 + 2;
	/* Initialize io_uring */
	io_uring_queue_init(depth, &ring, 0);
	int p[2];
	if (pipe(p) == -1) {
		perror("pipe");
	}

	for (int i = 1; i < argc; i++) {
		fprintf(stderr, "creating splice for %s\n", argv[i]);
		int fd = open(argv[i], O_RDONLY);
		if (fd == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
		io_uring_prep_splice(sqe, fd, -1, p[1], -1, UINT_MAX, 0);
		char *s;
		// leaking memory all over the place.
		asprintf(&s, "splice %s -> pipe", argv[i]);
		io_uring_sqe_set_data(sqe, s);
		sqe->flags |= IOSQE_IO_LINK;
		io_uring_submit(&ring);
		/* Don't worry about closing the fd. It's like, nbd y'all. */
	}
	/* And now a final splice to write from the pipe to stdout. */
	struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_splice(sqe, p[0], -1, STDOUT_FILENO, -1, UINT_MAX, 0);
	io_uring_sqe_set_data(sqe, "pipe -> stdout");
	io_uring_submit(&ring);

	for (int i = 1; i < argc + 1; i++) {
		struct io_uring_cqe *cqe;
		int ret = io_uring_wait_cqe(&ring, &cqe);
		if (ret < 0) {
			perror("io_uring_wait_cqe");
			exit(EXIT_FAILURE);
		}
		const char *s = io_uring_cqe_get_data(cqe);
		fprintf(stderr, "Operation %s\n", s);
		if (cqe->res < 0) {
			fprintf(stderr, "failed with res: %d\n\n", cqe->res);
			// exit(EXIT_FAILURE);
			continue;
		} else {
			fprintf(stderr, "succeded with res: %d\n\n", cqe->res);
		}
		io_uring_cqe_seen(&ring, cqe);
	}

	/* Call the clean-up function. */
	io_uring_queue_exit(&ring);
	exit(EXIT_SUCCESS);
}
