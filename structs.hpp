#pragma once

#include "utils/jstrings/jstrings.hpp"
#include <functional>
#include <map>
#include <string>

#define REGISTER_STRUCT(a, b, pragma_pack)

void init_structs();

enum VarType : int { Std, BField, Array };
struct Var {
    VarType type;
    uint8_t *data;
    size_t size;
    size_t sizeof_ctype;
    size_t offset;
    std::string name;
    std::function<void(const JStringList &)> set;
    std::function<void(const JStringList &)> print;
};

struct Struct {
    uint8_t *data = nullptr;
    size_t size = 0;
    std::string name;
    std::string type;
    std::function<void(const JStringList &)> print;
    std::map<std::string, Var> vars;
    size_t working_addr = 0;
};

struct StructFind {
    Struct *s = nullptr;
    Var *v = nullptr;
};
StructFind get_struct(std::string svname);

enum StructOperation { ERROR, READ_VAR, READ_STRUCT, WRITE, WRITE_BITFIELD, PASS };

struct StructParseOutput {
    enum StructOperation op = ERROR;
    Struct *s = nullptr;
    Var *v = nullptr;
    uint8_t *data = nullptr;
    size_t size = 0;
    size_t offset = 0;
};

StructParseOutput parse_struct_input(const JStringList &args);
