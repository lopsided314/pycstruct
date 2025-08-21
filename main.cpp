#include "structs.hpp"

#include <iostream>
#include <stdio.h>

#include <vector>

struct Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    float d[4];
    float f;
};

namespace Main {

struct Test2 {
    const char *str;
    double dd;
};
} // namespace Main

REGISTER_STRUCT(Test, test, -1);
REGISTER_STRUCT(Test2, test2, -1);
REGISTER_STRUCT(Name2, name, 4);


volatile uint8_t g_data[1024];

void print_buf() {
    for (size_t i = 0; i < 20; i++) {
        printf("%2lu, %8X\n", i*4, *(uint32_t*)&g_data[4*i]);
    }
}


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

    std::vector<JStringList> arg_sets = {
        {"mv", "name->", "4"},
        {"co", "name->plpl.att", "0xfff"},
        {"co", "name->plpl.phase", "0xfff"},
        {"mv", "name->", "0"},
        {"co", "name->a", "-1"},
        {"co", "name->d", "-1", "-.1", "-.01"},
        {"co", "name->e", "0xbbbb"},
        // {"co", "name->val", "0x111111"},
        {"co", "name->b", "123445678"},
        {"co", "name->c", "deadbeef"},
        {"ci", "name->"},
    };

    for (const JStringList &args : arg_sets) {

        StructParseOutput cmd = parse_struct_input(args);

        switch (cmd.op) {
        case READ_VAR:
            read(cmd.data, cmd.size, cmd.offset);
            cmd.v->print(args);
            break;

        case READ_STRUCT:
            read(cmd.data, cmd.size, cmd.offset);
            cmd.s->print(args);
            break;

        case WRITE:
            write(cmd.data, cmd.size, cmd.offset);
            continue;
            break;

        case WRITE_BITFIELD:
            read(cmd.data, cmd.size, cmd.offset);
            cmd.v->set(args);
            write(cmd.data, cmd.size, cmd.offset);
            continue;
            break;

        case ERROR:
            return 0;

        case PASS:
            std::cout << js::join(args, " ");
        default:
            break;
        }
        std::cout << "\n";
    }
}
