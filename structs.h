#pragma once

#include "jstrings.hpp"
#include <functional>
#include <map>
#include <string>

#define REGISTER_STRUCT(a, b)

void init_structs();
void print_var(std::string structname, std::string varname,
               JStringList extra_args = {});
void set_var(std::string structname, std::string varname,
             JStringList extra_args = {});

enum VarType : int { Std, BField, Array };

struct VarData {
    VarType type;
    void *data;
    size_t size;
};

VarData get_data(std::string structname, std::string varname);

struct Var {
    VarType type;
    uint8_t *data;
    size_t size;
    ssize_t offset;
    std::string name;
    std::function<void(JStringList)> set;
    std::function<void(JStringList)> print;
};

struct Struct {
    uint8_t *data;
    size_t size;
    std::string name;
    std::string type;
    std::function<void(JStringList)> print;
    std::map<std::string, Var> vars;
    size_t working_addr;
};

struct StructFind {
    Struct *s;
    Var *v;
};

StructFind get_struct(std::string structname, std::string varname);

struct StructParseOutput {
    enum Operation {
        ERROR,
        PRINT,
        SET,
        SET_BITFIELD,
        SET_ARRAY,
        PASS
    } op;
    Struct *s;
    Var *v;
    uint8_t *data;
    size_t size;
};

//Current: parse function that returns this shit
