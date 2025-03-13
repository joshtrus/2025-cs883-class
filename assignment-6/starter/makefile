# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g

# Target executable name
TARGET = dsh

# Find all source and header files
SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)

# Default target
all: $(TARGET)

# Compile source to executable; link with pthread for multi-threaded functionality.
$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lpthread

# Clean up build files
clean:
	rm -f $(TARGET)

# Run all BATS tests in the ./bats directory.
test:
	bats $(wildcard ./bats/*.sh)

# Run valgrind for memory leak and thread error checks.
valgrind:
	echo "pwd\nexit" | valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TARGET)
	echo "pwd\nexit" | valgrind --tool=helgrind --error-exitcode=1 ./$(TARGET)

# Phony targets
.PHONY: all clean test 