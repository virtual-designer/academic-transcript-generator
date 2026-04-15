CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -Wno-unused-parameter -std=gnu99 -O2
RM = rm -f

BIN = atsgen

all:
	@echo Please use 'make build-<platform>' && exit 1

build-linux:
	$(CC) $(CFLAGS) src/$(BIN).c -lhpdf -o $(BIN)

clean:
	$(RM) $(BIN)

.PHONY: all clean build
