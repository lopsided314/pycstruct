CXX:=g++
STD:=-std=c++11
FLAGS:=-Wpedantic -Wextra -Wall -ggdb -O1 
INCLUDES:=-I.
DEFINES:=-DCOMPILE_EPOCH=$(shell date +%s)
EXE=test
PYTHON=python3 

.PHONY: all
all:
	python3 pycstruct3.py $(INCLUDES)
	$(CXX) $(FLAGS) $(STD) $(INCLUDES) $(DEFINES) structs.cpp main.cpp -o $(EXE)

.PHONY: run
run:
	./$(EXE)

