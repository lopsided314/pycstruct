#include "structs.hpp"

#include <iostream>
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

REGISTER_PYCSTRUCT(Test, test);
REGISTER_PYCSTRUCT(Test2, test2);
REGISTER_PYCSTRUCT_PACK(Name2, name, 4);

namespace js = JStrings;

volatile uint8_t g_data[1024];

void print_buf() {
    for (size_t i = 0; i < 20; i++) {
        std::cout << js::fmt("%d, %8X\n", i * 4, *(uint32_t *)&g_data[4 * i]);
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

int main() {
    using namespace Structs;

    init_structs();

    std::vector<JStringList> arg_sets = {
        // {"mv", "name->", "4"},
        // {"co", "name->plpl.att", "0xfff"},
        // {"co", "name->plpl.phase", "0xfff"},
        // {"mv", "name->", "0"},
        {"co", "name->a", "-1"},
        // {"co", "name->d", "-1", "-.1", "-.01"},
        {"co", "name->e", "0xabbbb"},
        // {"co", "name->val", "0x111111"},
        // {"co", "name->b", "123445678"},
        // {"co", "name->c", "deadbeef"},
        {"co", "name->u1.f", "-3.1415"},
        // {"ci", "name->"},
        // {"ci", "name->plpl*"},
        // {"mv", "name->", "100"},
        // {"ci", "name->"},
        // {"mv", "name->", "00"},
        // {"co", "name->d[1]", "-3.1415"},
        // {"co", "name->str", "some",  "text"},
        {"ci", "name->"},
        // {"struct_src", "name->"},
    };

    for (const JStringList &args : arg_sets) {

        if (is_struct_cmd(args)) {

            const auto &cmd = parse_struct_cmd(args);

            switch (cmd.op) {
            case Structs::OpReq::PRINT:
                read(cmd.data, cmd.size, cmd.offset);
                cmd.print(args);
                break;

            case Structs::OpReq::READ_WRITE:
                read(cmd.data, cmd.size, cmd.offset);
                // fall through
            case Structs::OpReq::WRITE:
                cmd.set_val(args);
                write(cmd.data, cmd.size, cmd.offset);
                break;

            case Structs::OpReq::PASS:
                std::cout << "pass: " << js::join(args, " ") << "\n";
                break;

            case Structs::OpReq::ERROR:
                std::cout << "error\n";
                return 0;
            }

            std::cout << "\n";
        }
    }
}
