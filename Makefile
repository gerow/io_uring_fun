SRCS = $(wildcard *.c)
BINS = $(SRCS:.c=)

CC = cc

CFLAGS = -Iliburing/src/include
LDFLAGS = -Lliburing/src
LIBS    = -luring

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

all: $(BINS) .gitignore

clean:
	rm -f $(BINS)

.gitignore: Makefile
	echo $(BINS) | tr ' ' '\n' >.gitignore
