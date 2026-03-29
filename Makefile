CXX=g++
STD=-std=c++11
FLAGS=-Wpedantic -Wextra -Wall -ggdb -O1 
INCLUDES=-I./utils/fmt/include/
EXE=test
PYTHON=python3 


all:
	python3 pycstruct3.py $(INCLUDES)
	$(CXX) $(FLAGS) $(STD) $(INCLUDES) structs.cpp main.cpp ./utils/fmt/format.o -o $(EXE)
