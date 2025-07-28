#include "structs.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <string>

struct Var {
    void *data;
    size_t size;
    size_t offset;
    std::string name;
    std::function<void(std::string)> set;
    std::function<void(void)> print;
};

// struct Bitfield_field {
//     std::string name;
//     std::function<void(std::string)> set;
//     std::function<void(void)> print;
// };

struct Struct {
    void *data;
    size_t size;
    std::string name;
    std::string type;
    std::function<void(void)> print;
    std::map<std::string, Var> vars;
    // std::map<std::string, Bitfield_field> bfields;
};

std::map<std::string, Struct> g_structs;

const std::map<std::string, std::string> printf_fmts{{"i32", "%10d"}};
#define REGISTER_INTERNAL_STRUCT(structtype, structname)                       \
    g_structs[#structname] =                                                   \
        Struct{.data = &structname,                                            \
               .size = sizeof(structname),                                     \
               .name = #structname,                                            \
               .type = #structtype,                                            \
               .print = [](void) { printf(#structname " print\n"); },          \
               .vars = {},                                                     \
               .bfields = {}};

#define REGISTER_VAR(structname, varname, ctype, printf_fmt)                   \
    assert(g_structs.count(#structname) &&                                     \
           "Trying to register var with unregistered struct");                 \
    g_structs[#structname].vars[#varname] =                                    \
        Var{.data = &structname.varname,                                       \
            .size = sizeof(structname.varname),                                \
            .offset = (size_t)&structname.varname - (size_t)&structname,       \
            .name = #varname,                                                  \
            .set =                                                             \
                [](std::string arg) {                                          \
                    ctype val = std::stol(arg);                                \
                    structname.varname = val;                                  \
                },                                                             \
            .print =                                                           \
                [](void) {                                                     \
                    printf(#structname "->" #varname " = " printf_fmt "\n",    \
                           structname.varname);                                \
                }};

#define REGISTER_BITFIELD(structname, varname, ctype, printf_fmt)              \
    assert(g_structs.count(#structname) &&                                     \
           "Trying to register var with unregistered struct");                 \
    g_structs[#structname].vars[#varname] =                                    \
        Var{.data = &structname,                                               \
            .size = sizeof(structname),                                        \
            .offset = 0,                                                       \
            .name = #varname,                                                  \
            .set =                                                             \
                [](std::string arg) {                                          \
                    ctype val = std::stol(arg);                                \
                    structname.varname = val;                                  \
                },                                                             \
            .print =                                                           \
                [](void) {                                                     \
                    printf(#structname "->" #varname " = " printf_fmt "\n",    \
                           structname.varname);                                \
                }};

void print_var(std::string structname, std::string varname) {
    if (g_structs.count(structname) &&
        g_structs[structname].vars.count(varname)) {
        g_structs[structname].vars[varname].print();
    } else {
    }
}

struct StructData {
    void *data;
    size_t size;
};

StructData get_data(std::string structname, std::string varname) {

    if (g_structs.count(structname)) {
        if (g_structs[structname].vars.count(varname)) {
            return StructData{
                .data = g_structs[structname].vars[varname].data,
                .size = g_structs[structname].vars[varname].size,
            };
        }
    }

    return StructData{.data = nullptr, .size = 0};
}

/**
 * EVERYTHING IN THIS FILE AFTER THIS LINE WILL BE OVERWRITTEN
 */

// pycstruct_shit
static struct _Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    long long d;
    float f;
} test;

void init_structs() {
    REGISTER_INTERNAL_STRUCT(_Test, test)
    REGISTER_VAR(test, a, int, "%9d")
    REGISTER_VAR(test, d, long long, "%20lld")
    REGISTER_VAR(test, f, float, "%14.4e")
    REGISTER_BITFIELD(test, b, unsigned int, "%08X")
    REGISTER_BITFIELD(test, c, unsigned int, "%08X")
}

void set_var(std::string structname, std::string varname) {
    if (structname == "test" && varname == "a") {
    }
    if (structname == "test" && varname == "d") {
    }
    if (structname == "test" && varname == "f") {
    }
    if (structname == "test" && varname == "b") {
    }
    if (structname == "test" && varname == "c") {
    }
}
