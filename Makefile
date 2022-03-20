CXX = g++-10
CXX_FLAGS = -O2 -march=native -Wall -Wextra -std=c++20 -Iinclude -Ibin

all: bin/TST8080 bin/CPUTEST bin/8080PRE bin/8080EXM bin/invaders

bin/%: bin/%.o bin/emulator.o
	${CXX} -o $@ $^

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

bin/invaders: bin/invaders.o bin/emulator.o
	${CXX} -o $@ $^ $(shell sdl2-config --libs)

bin/invaders.o: src/invaders.cpp include/emulator.h bin/invaders.h
	${CXX} ${CXX_FLAGS} $(shell sdl2-config --cflags) -c -o $@ $<

bin/invaders.h: roms/invaders.h roms/invaders.g roms/invaders.f roms/invaders.e
	xxd -i roms/invaders.h > bin/invaders.h
	xxd -i roms/invaders.g >> bin/invaders.h
	xxd -i roms/invaders.f >> bin/invaders.h
	xxd -i roms/invaders.e >> bin/invaders.h

bin/%.o: src/%.cpp include/%.h
	${CXX} ${CXX_FLAGS} -c -o $@ $<

bin/%.o: src/%.cpp
	${CXX} ${CXX_FLAGS} -c -o $@ $<

clean:
	rm bin/*

.PHONY: all clean