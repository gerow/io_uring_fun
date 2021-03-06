#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// dumb way to validate that, yes, reading from an empty pipe blocks forever.
int main(int argc, char **argv)
{
	int p[2];
	if (pipe(p) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	char buf[4096];
	ssize_t br = read(p[0], buf, sizeof(buf));
	if (br == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "read returned %d\n", br);
	exit(EXIT_SUCCESS);
}
