CXX = g++-10
CXX_FLAGS = -Og -ggdb3 -Wall -Wextra -std=c++20 -Iinclude

all: bin/asm8080 bin/tst8080

bin/asm8080: bin/asm8080.o bin/assembler.o
	${CXX} ${CXX_FLAGS} -o $@ $^

bin/tst8080: bin/tst8080.o bin/emulator.o
	${CXX} ${CXX_FLAGS} -o $@ $^

bin/%.o: src/%.cpp include/%.h
	${CXX} ${CXX_FLAGS} -c -o $@ $<

bin/%.o: src/%.cpp
	${CXX} ${CXX_FLAGS} -c -o $@ $<

bin/%.o: test/%.cpp
	${CXX} ${CXX_FLAGS} -c -o $@ $<

clean:
	rm bin/*

.PHONY: all clean