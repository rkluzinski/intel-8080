CXX = g++
CXX_FLAGS = -Og -ggdb3 -Wall -Wextra -std=c++2a

all: bin/main

bin/main: bin/main.o
	${CXX} ${CXX_FLAGS} -o $@ $^

bin/main.o: src/main.cpp
	${CXX} ${CXX_FLAGS} -c -o $@ $<

.PHONY: all