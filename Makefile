CXX = g++-10
CXX_FLAGS = -O2 -march=native -ggdb3 -Wall -Wextra -std=c++20 -Iinclude -Ibin

all: bin/TST8080 bin/CPUTEST bin/8080PRE bin/8080EXM

bin/%: bin/%.o bin/emulator.o
	${CXX} ${CXX_FLAGS} -o $@ $^

bin/8080PRE.o: test/main.cpp bin/8080PRE.h bin/BDOS.h
	${CXX} ${CXX_FLAGS} -D_8080PRE -c -o $@ $<

bin/8080EXM.o: test/main.cpp bin/8080EXM.h bin/BDOS.h
	${CXX} ${CXX_FLAGS} -D_8080EXM -c -o $@ $<

bin/%.o: test/main.cpp bin/%.h bin/BDOS.h
	${CXX} ${CXX_FLAGS} -D$* -c -o $@ $<

bin/%.h: coms/%.COM
	xxd -i $< $@

bin/BDOS.h: bin/asm test/BDOS.ASM
	bin/asm test/BDOS.ASM bin/BDOS
	xxd -i bin/BDOS bin/BDOS.h

bin/asm: bin/asm.o bin/assembler.o
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