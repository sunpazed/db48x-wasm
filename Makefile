# Variables
CXX := emcc
CXXFLAGS := $(CFLAGS) 
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := $(BINDIR)/main.js
HELPFILE_NAME=\"/help/db48x.md\"


C_DEFS := -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))"

# List of individual source files to include
FILES :=

# Recursively find all .cc files in the src directory
SRCS := $(shell find $(SRCDIR) -type f -name '*.cc') $(patsubst %, $(SRCDIR)/%, $(FILES))

# Convert source files to object files in the build directory
OBJS := $(patsubst $(SRCDIR)/%.cc, $(BUILDDIR)/%.o, $(SRCS))

# emcc flags
CXXFLAGS += -s MODULARIZE=0 \
 -s EXPORT_NAME='Module' \
 -s RESERVED_FUNCTION_POINTERS=20 \
 --bind 
# -s WASM=1 \
#   -s ALLOW_MEMORY_GROWTH=1 \
#   -s BINARYEN_ASYNC_COMPILATION=0 \
#   -s SINGLE_FILE=1 \
#   -s MODULARIZE=1 \

# Default target
all: $(TARGET)

# Include directories
INCDIR := include 
INCLUDES := $(patsubst %, -I%, $(INCDIR))

# Add include directories to CXXFLAGS
CXXFLAGS += $(INCLUDES) $(C_DEFS) 

# Rule to link the final executable
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .cc files to .o files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build and bin directories
clean:
	rm -rf $(BUILDDIR)/*.o $(TARGET)

# Phony targets
.PHONY: all clean
