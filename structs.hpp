#pragma once

#include "utils/jstrings/jstrings.hpp"

#define REGISTER_PYCSTRUCT(typename, instance_name)
#define REGISTER_PYCSTRUCT_PACK(typename, instance_name, pragma_pack)

namespace Structs {

void init_structs();

enum StructOperation { ERROR, PRINT, WRITE, READ_WRITE, PASS };

struct OpReq {
    uint8_t *data = nullptr;
    size_t size = 0;
    size_t offset = 0;
    void (*set_val)(const JStringList &) = nullptr;
    void (*print)(const JStringList &) = nullptr;
    enum class Op { ERROR, PRINT, WRITE, READ_WRITE, PASS } op = Op::ERROR;
};

const OpReq &parse_struct_cmd(const JStringList &args);

JStringList struct_names();
JStringList member_names(const std::string &arg);

} // namespace Structs
