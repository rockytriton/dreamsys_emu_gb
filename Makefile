CC=g++
CFLAGS=-Werror=all -std=c++17 -g
DEPS = src/common.h

SRC_DIR := src
OBJ_DIR := obj

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: emu

emu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lSDL2 -lpthread

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -f -r $(OBJ_DIR)


