CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=gnu99 -O2
RM = rm -f

BIN = src/atsgen

all: $(BIN)
$(BIN):
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	$(RM) $(BIN)

.PHONY: all clean
