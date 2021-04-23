CC = gcc
LDFLAGS = -lsequoia -ljack -ljson-c

BIN_DIR = bin

SOURCES := $(wildcard *.c)
BINS :=  $(patsubst %.c,$(BIN_DIR)/%,$(SOURCES))

.PHONY: default clean

default: $(BIN_DIR) $(BINS)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BIN_DIR)/%: %.c
	$(CC) -o $@ $? $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)
