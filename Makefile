CC=g++
CFLAGS=-Werror=all -std=c++17
DEPS = src/common.h

SRC_DIR := src
OBJ_DIR := obj

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

#%.o: %.cpp $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

#OBJ=bus.o cart.o cpu.o emu.o io.o main.o memory.o op_handler.o opcodes.o ppu.o ui.o

all: emu

emu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lSDL2 -lpthread

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -f -r $(OBJ_DIR)


