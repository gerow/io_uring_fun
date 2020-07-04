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
	int depth = 1024;
	/* Initialize io_uring */
	io_uring_queue_init(depth, &ring, 0);
	int p[2];
	if (pipe(p) == -1) {
		perror("pipe");
	}

	/* create a close that will fail, and hopefully cancel everyting else */
	struct io_uring_sqe *sqe;
	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_close(sqe, -1);
	sqe->flags |= IOSQE_IO_LINK;
	io_uring_sqe_set_data(sqe, "close(-1)");

	for (int i = 1; i < argc; i++) {
		fprintf(stderr, "creating splice for %s\n", argv[i]);
		int fd = open(argv[i], O_RDONLY);
		if (fd == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}

		char *s;
		asprintf(&s, "splice %s -> pipe", argv[i]);

		sqe = io_uring_get_sqe(&ring);
		io_uring_prep_splice(sqe, fd, -1, p[1], -1, 131072,
				     SPLICE_F_MOVE);
		// leaking memory all over the place.
		io_uring_sqe_set_data(sqe, s);
		//sqe->flags |= IOSQE_IO_LINK | IOSQE_IO_HARDLINK;
		/* Don't worry about closing the fd. It's like, nbd y'all. */
	}
	/* When we're done close the write side of the pipe. */
	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_close(sqe, p[1]);
	io_uring_sqe_set_data(sqe, "close write side of pipe");
	//sqe->flags |= IOSQE_IO_LINK | IOSQE_IO_HARDLINK;

	/* And now a final splice to write from the pipe to stdout. */
	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_splice(sqe, p[0], -1, STDOUT_FILENO, -1, 131072,
			     SPLICE_F_MOVE);
	io_uring_sqe_set_data(sqe, "pipe -> stdout");
	io_uring_submit(&ring);

	for (;;) {
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
		} else {
			fprintf(stderr, "succeded with res: %d\n\n", cqe->res);
		}
		io_uring_cqe_seen(&ring, cqe);
	}

	/* Call the clean-up function. */
	io_uring_queue_exit(&ring);
	exit(EXIT_SUCCESS);
}
