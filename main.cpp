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

int main() {
    init_structs();

    set_var("test2", "dd", {".40"});
    print_var("test2", "dd");

    set_var("test", "d", {"-1", "-2", "-3"});
    print_var("test", "d", {"1"});

    uint8_t data[0x1000];

    // com test-> 0x100
    // co test->c 0xff
    // ci test->c

    auto sv = get_struct("test", "c");

    JStringList args = {"0xfb"};

    if (sv.s && sv.v) {

        if (sv.v->type == VarType::BField) {
            read(sv.v->data, sv.v->offset + sv.s->working_addr, sv.v->size);
        }
        sv.v->set(args);
        write(sv.v->data, sv.v->offset + sv.s->working_addr, sv.v->size);
    }
}
