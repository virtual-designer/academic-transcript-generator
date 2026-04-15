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

build-windows:
	$(CC) $(CFLAGS) -Isrc -Iinclude -L. src/$(BIN).c -lhpdf -o $(BIN).exe

run-linux: build-linux
	./$(BIN)
	
run-darwin: build-darwin
	./$(BIN)

run-windows: build-windows
	./$(BIN).exe

clean:
	$(RM) $(BIN)

.PHONY: all clean build-linux build-darwin build-windows run-linux run-darwin run-windows
