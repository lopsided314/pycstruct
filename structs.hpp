#pragma once

#include "utils/jstrings/jstrings.hpp"

#define REGISTER_STRUCT(typename, instance_name)
#define REGISTER_STRUCT_PACK(typename, instance_name, pragma_pack)

namespace Structs {

void init_structs();

enum StructOperation { ERROR, PRINT, WRITE, READ_WRITE, PASS };

struct StructCmdFeedback {
    enum StructOperation op = ERROR;
    uint8_t *data = nullptr;
    size_t size = 0;
    size_t offset = 0;
    void (*set_val)(const JStringList &);
    void (*print)(const JStringList &);
};

StructCmdFeedback parse_struct_cmd(const JStringList &args);

} // namespace Structs
