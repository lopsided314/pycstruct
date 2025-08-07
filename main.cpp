#include "jstrings.hpp"
#include "structs.h"

#include <iostream>

struct Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    long long d[4];
    float f;
};

struct Test2 {
    const char *str;
    double dd;
};

REGISTER_STRUCT(Test, test);
REGISTER_STRUCT(Test2, test2);

void write(uint8_t *, size_t , size_t ) {}
void read(uint8_t *, size_t , size_t ) {}

namespace js = JStrings;
int main() {
    init_structs();

}
