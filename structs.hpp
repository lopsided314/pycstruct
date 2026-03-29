#pragma once

#include "utils/jstrings/jstrings.hpp"
#include <map>
#include <memory>
#include <string>

#define REGISTER_STRUCT(typename, instance_name)
#define REGISTER_STRUCT_PACK(typename, instance_name, pragma_pack)

namespace Structs {

void init_structs();

enum VarType : int { Std, BField, Array };
struct Var {
    VarType type;
    uint8_t *data;
    size_t size;
    size_t sizeof_ctype;
    size_t offset;
    std::string name;
    void (*set)(const JStringList &);
    void (*print)(const JStringList &);
};

struct Struct {
    uint8_t *data = nullptr;
    size_t size = 0;
    std::string name;
    std::string type;
    std::string src_filepath;
    std::string src_definition;
    void (*print)(const JStringList &);
    std::map<std::string, std::unique_ptr<Var>> vars;
    size_t working_addr = 0;
};

struct StructFind {
    Struct *s = nullptr;
    Var *v = nullptr;
};
StructFind get_struct(std::string svname);

enum StructOperation { ERROR, READ_VAR, READ_STRUCT, WRITE, READ_WRITE, PASS };

struct StructParseOutput {
    enum StructOperation op = ERROR;
    Struct *s = nullptr;
    Var *v = nullptr;
    uint8_t *data = nullptr;
    size_t size = 0;
    size_t offset = 0;
};

StructParseOutput parse_struct_input(const JStringList &args);
} // namespace Structs
