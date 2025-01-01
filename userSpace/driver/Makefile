# Makefile for Producer-Consumer Deque Project

# Compiler and Flags
CC = gcc
CFLAGS = -pthread -Wall -Wextra

# Executable Name
TARGET = memefs

# Source Files
SRC = memefs.c

# Object Files
OBJ = $(SRC:.c=.o)

# Default Target
all: $(TARGET)

# Rule to Create Executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Rule to Compile Source Files into Object Files
%.o: %.c deque.h
	$(CC) $(CFLAGS) -c $<

# Clean Up Compiled Files
clean:
	rm -f $(OBJ) $(TARGET)

# Run the Program
run: $(TARGET)
	./$(TARGET)
