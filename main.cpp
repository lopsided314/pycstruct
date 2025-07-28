#include "structs.h"

#include <iostream>

struct Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    long long d;
    float f;
};

REGISTER_STRUCT(Test, test);

int main() {
    init_structs();

    print_var("test", "d");
}

/*
 * take in input from command line that is determined to either be a read or
 write
 * determine the struct and member name from parsing the input
 *
 * if write function:
 *      if bitfield:
 *          read full size of struct from memory into local struct
 *      turn input into correct data type
        set local struct member to value
        if bitfield:
 *          return the address and size of the local struct
 *      else:
 *          return the address and size of the member variable
 *      write those bytes to whatever
 *
 *
 * */
