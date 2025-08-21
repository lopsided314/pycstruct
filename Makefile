CXX=g++
STD=-std=c++11
FLAGS=-Wpedantic -Wextra -Wall -ggdb -O1 
INCLUDES=
EXE=test
PYTHON=python3 


all:
	python3 pycstruct2.py $(INCLUDES)
	$(CXX) $(FLAGS) $(STD) structs.cpp main.cpp -o $(EXE)
