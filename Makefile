CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -Wno-unused-parameter -std=gnu99 -O2
RM = rm -f

BIN = atsgen

all:
	@echo Please use 'make build-<linux|darwin|windows>' && exit 1

build-linux:
	$(CC) $(CFLAGS) src/$(BIN).c -lhpdf -o $(BIN)

build-darwin:
	$(CC) $(CFLAGS) -I/opt/homebrew/include -L/opt/homebrew/lib src/$(BIN).c -lhpdf -o $(BIN)

clean:
	$(RM) $(BIN)

.PHONY: all clean build-linux build-darwin
