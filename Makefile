BINS = splicecat

CC = cc

CFLAGS = -Iliburing/src/include
LDFLAGS = -Lliburing/src
LIBS    = -luring

#%.o: %.c
#	$(CC) -c -Iliburing/src/include $< -o $@

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

all: $(BINS)

clean:
	rm -f *.o $(BINS)
