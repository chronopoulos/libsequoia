CC = gcc
CFLAGS += -Wall -O2

# inputs

SRC_DIR = src
INC_DIR = include

SOURCES := $(wildcard $(SRC_DIR)/*.c)

# outputs

LIB_DIR = lib
SO = $(LIB_DIR)/libgenseq.so
OBJS :=  $(patsubst $(SRC_DIR)/%.c,$(LIB_DIR)/%.o,$(SOURCES))

INSTALL_DIR_LIB = /usr/local/lib
INSTALL_DIR_INC = /usr/local/include

##########################

.PHONY: all library examples clean install uninstall

##########################

default: library

library: $(SO)

##########################

$(SO): $(LIB_DIR) $(OBJS)
	$(CC) $(CFLAGS) -shared -o$@ $(OBJS)

$(LIB_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -fPIC -I$(INC_DIR) -o$@ $?

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

##########################

install: $(SO)
	sudo cp $< $(INSTALL_DIR_LIB)
	sudo cp -r $(INC_DIR)/* $(INSTALL_DIR_INC)

uninstall:
	sudo rm $(INSTALL_DIR_LIB)/$(SO) # TODO fix this

clean:
	rm -rf $(LIB_DIR)
