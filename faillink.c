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

int main(int argc, char *argv[])
{
	struct io_uring ring;
	int depth = 1024;
	io_uring_queue_init(depth, &ring, 0);

	struct io_uring_sqe *sqe;
	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_nop(sqe);
	sqe->flags |= IOSQE_IO_LINK;

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
