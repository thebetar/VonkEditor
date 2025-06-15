# Makefile

SRC = ./src/*.cpp
OUT = ./main
CFLAGS = -Wall -g

# Default target
all:
		g++ $(CFLAGS) $(SRC) -o $(OUT) -lncurses

# Clean target
clean:
		rm -f $(OUT)
