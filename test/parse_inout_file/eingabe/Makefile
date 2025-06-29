# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# entry point for the program and target name
MAIN := module.c


# Additional flags for the compiler
CFLAGS := 

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Determine if clang or gcc is available
CC := $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor gcc is available. Exiting.)
endif

# all source files needed for the program
SOURCES := $(MAIN)
TARGET := $(basename $(MAIN))

# Default to release build for both app and library
all: debug

# Debug build
debug: CFLAGS += -g
debug: $(TARGET)

# Release build
release: CFLAGS += -O2
release: $(TARGET)

# recipe for building the program
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

# clean up
clean:
	rm -f $(TARGET)

.PHONY: all debug release clean