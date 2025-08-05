CXX=g++
STD=-std=c++11
FLAGS=-Wextra -Wall -ggdb -fsanitize=address 
EXE=test
PYTHON=python3 


all:
	python3 pycstruct2.py
	$(CXX) $(FLAGS) $(STD) structs.cpp main.cpp -o $(EXE)
