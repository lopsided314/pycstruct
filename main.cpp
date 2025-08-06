#include "structs.h"

#include <iostream>

struct Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    long long d[4];
    float f;
};

struct Test2 {
    const char* str;
    double dd;
};

REGISTER_STRUCT(Test, test);
REGISTER_STRUCT(Test2, test2);

int main() {
    init_structs();

    set_var("test2", "dd", {".40"});
    print_var("test2", "dd");


    set_var("test", "d", {"-1", "-2", "-3"});
    print_var("test", "d", {"1"});

    uint8_t data[0x1000];



    auto sd = get_data("test", "c");



}
