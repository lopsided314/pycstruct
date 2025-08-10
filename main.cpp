#include "jstrings.hpp"
#include "structs.h"

#include <iostream>
#include <stdio.h>

#include <vector>
#pragma pack(0)
struct Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    float d[4];
    float f;
};

struct Test2 {
    const char *str;
    double dd;
};

REGISTER_STRUCT(Test, test);
REGISTER_STRUCT(Test2, test2);

uint8_t g_data[1024];

void write(uint8_t *data, size_t size, size_t offset) {
    // printf("  write: %p %lu\n", data, size);
    memcpy((void *)(g_data + offset), data, size);
}
void read(uint8_t *data, size_t size, size_t offset) {
    // printf("  read : %p %lu\n", data, size);
    memcpy(data, (void *)(g_data + offset), size);
}
namespace js = JStrings;

int main() {
    init_structs();

    std::vector<JStringList> arg_sets = {{"co", "test->d[0]", "-14"},
                                         {"ci", "test->a"},
                                         {"ci", "test->d[0,2]"},
                                         {"co", "test->c", "0xabcd"},
                                         {"ci", "test->c"},
                                         {"ci", "test->d"},
                                         {"co", "test->d[1:4]", "3.34e-5", "12", "13"},
                                         {"ci", "test->d"},
                                         {"mv", "test->", "8"},
                                         {"co", "test->a", "-1"},
                                         {"mv", "test->", "0"},
                                         {"ci", "test->d"}};

    for (const JStringList &args : arg_sets) {

        StructParseOutput cmd = parse_struct_input(args);

        switch (cmd.op) {
        case READ:
            read(cmd.data, cmd.size, cmd.offset + cmd.s->working_addr);
            cmd.v->print(args);
            break;

        case WRITE:
            write(cmd.data, cmd.size, cmd.offset + cmd.s->working_addr);
            break;

        case WRITE_BITFIELD:
            read(cmd.data, cmd.size, cmd.offset + cmd.s->working_addr);
            cmd.v->set(args);
            write(cmd.data, cmd.size, cmd.offset + cmd.s->working_addr);
            break;

        case ERROR:
            return 0;

        case PASS:
                std::cout << js::join(args, " ") << "\n";
        default:
            break;
        }
        std::cout << "\n";
    }
}
