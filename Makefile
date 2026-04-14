# ================================================================
#  Makefile — Pathfinding Visualizer (macOS + Raylib)
# ================================================================
#  Usage:
#    make          Build the project
#    make run      Build and run
#    make clean    Remove compiled files
#
#  Prerequisites:
#    brew install raylib
#
#  How it works:
#    - $(shell brew --prefix raylib) finds Raylib's install path
#      (works on both Intel and Apple Silicon Macs)
#    - We link against Raylib + required macOS frameworks
# ================================================================

# Compiler and flags
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -I$(shell brew --prefix raylib)/include
LDFLAGS  = -L$(shell brew --prefix raylib)/lib -lraylib \
           -framework OpenGL -framework IOKit \
           -framework Cocoa -framework CoreVideo

# Output binary name
TARGET   = pathfinder

# Source files (add new .c files here as we build more modules)
SRCS     = main.c grid.c renderer.c priority_queue.c pathfinder.c
OBJS     = $(SRCS:.c=.o)

# ---- TARGETS ----

# Default: compile everything
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile each .c file into a .o object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build and run in one step
run: $(TARGET)
	./$(TARGET)

# Remove all build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Mark these as "not real files" so Make doesn't get confused
.PHONY: all run clean
