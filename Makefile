CXX=g++
STD=-std=c++11
FLAGS=-Wextra -Wall -ggdb -fsanitize=address 
EXE=test
PYTHON=python3 


all:
	$(CXX) $(FLAGS) $(STD) structs.cpp main.cpp -o $(EXE)
	# python3 pycstruct2.py
