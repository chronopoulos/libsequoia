PYTHON = python

# inputs

SETUP_PY = setup.py
SOURCES := $(wildcard *.c) $(wildcard *.h)

# outputs

BUILD_DIR = build
FILE_REC = installed_files

##########################

.PHONY: clean install uninstall

##########################

$(BUILD_DIR): $(SOURCES)
	$(PYTHON) $(SETUP_PY) build

clean:
	rm -rf $(BUILD_DIR) $(FILE_REC)

install: $(BUILD_DIR)
	sudo $(PYTHON) $(SETUP_PY) install --record $(FILE_REC)

uninstall:
	sudo rm `cat $(FILE_REC)`
