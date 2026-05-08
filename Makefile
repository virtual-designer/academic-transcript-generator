CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c99 -O2 $(CFLAGS_EXTRA)
RM = rm -f

BIN = atsgen

all:
	@echo Please use 'make build-<linux|darwin|windows>' && exit 1

build-linux:
	$(CC) $(CFLAGS) src/$(BIN).c -o $(BIN)

build-darwin:
	$(CC) $(CFLAGS) src/$(BIN).c -o $(BIN)

build-windows:
	$(CC) $(CFLAGS) src/$(BIN).c -o $(BIN).exe

run-linux: build-linux
	./$(BIN)

run-darwin: build-darwin
	./$(BIN)

run-windows: build-windows
	./$(BIN).exe

clean:
	$(RM) $(BIN)

.PHONY: all clean build-linux build-darwin build-windows run-linux run-darwin run-windows
