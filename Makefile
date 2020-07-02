BINS = splicecat

CC = cc

CFLAGS = -Iliburing/src/include
LDFLAGS = -Lliburing/src
LIBS    = -luring

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

all: $(BINS)

clean:
	rm -f *.o $(BINS)
