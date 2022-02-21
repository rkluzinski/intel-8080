CXX = g++-10
CXX_FLAGS = -Og -ggdb3 -Wall -Wextra -std=c++20

all: bin/assembler bin/emulator

bin/assembler: bin/assembler.o
	${CXX} ${CXX_FLAGS} -o $@ $^

bin/emulator: bin/emulator.o
	${CXX} ${CXX_FLAGS} -o $@ $^

bin/%.o: src/%.cpp
	${CXX} ${CXX_FLAGS} -c -o $@ $<

clean:
	rm bin/*

.PHONY: all clean