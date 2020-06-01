CC=g++
CFLAGS=-O2 -Werror=all -std=c++17
DEPS = common.h

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

OBJ=bus.o cart.o cpu.o emu.o io.o main.o memory.o op_handler.o opcodes.o ppu.o ui.o

emu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lSDL2 -lpthread

all: emu


clean:
	rm -f *.o emu
