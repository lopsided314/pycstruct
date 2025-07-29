#include "structs.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

struct Var {
    void *data;
    size_t size;
    size_t offset;
    std::string name;
    std::function<void(std::string)> set;
    std::function<void(std::string)> print;
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
    std::function<void(std::string)> print;
    std::map<std::string, Var> vars;
    // std::map<std::string, Bitfield_field> bfields;
};

std::map<std::string, Struct> g_structs;

static unsigned long stoul_0x(std::string arg, bool *ok = nullptr) {
    if (ok)
        *ok = false;
    size_t i;
    unsigned long val = 0;
    try {
        val = std::stoul(arg, &i, 16);
    } catch (const std::exception &e) {
        return 0;
    }
    if (i != arg.length()) {
        return 0;
    }
    if (ok)
        *ok = true;
    return val;
}

static long stol(std::string arg, bool *ok = nullptr) {
    if (ok)
        *ok = false;
    size_t i;
    long val = 0;
    try {
        val = std::stol(arg, &i);
    } catch (const std::exception &e) {
        return 0;
    }
    if (i != arg.length()) {
        return 0;
    }
    if (ok)
        *ok = true;
    return val;
}

static double stod(std::string arg, bool *ok = nullptr) {
    if (ok)
        *ok = false;
    size_t i;
    double val = 0;
    try {
        val = std::stod(arg, &i);
    } catch (const std::exception &e) {
        return 0;
    }
    if (i != arg.length()) {
        return 0;
    }
    if (ok)
        *ok = true;
    return val;
}

std::vector<std::string> split(std::string) { return {}; }
void replace(std::string *) {}

const std::map<std::string, std::string> printf_fmts{{"i32", "%10d"}};
#define REGISTER_INTERNAL_STRUCT(structtype, structname)                       \
    g_structs[#structname] = Struct{.data = &structname,                       \
                                    .size = sizeof(structname),                \
                                    .name = #structname,                       \
                                    .type = #structtype,                       \
                                    .print =                                   \
                                        [](std::string args) {                 \
                                            (void)args;                        \
                                            printf(#structname " print\n");    \
                                        },                                     \
                                    .vars = {}};

#define REGISTER_VAR(structname, varname, ctype, printf_fmt, stonum)           \
    assert(g_structs.count(#structname) &&                                     \
           "Trying to register var with unregistered struct: " #structname);   \
    g_structs[#structname].vars[#varname] =                                    \
        Var{.data = &structname.varname,                                       \
            .size = sizeof(structname.varname),                                \
            .offset = (size_t)&structname.varname - (size_t)&structname,       \
            .name = #varname,                                                  \
            .set =                                                             \
                [](std::string arg) {                                          \
                    bool ok;                                                   \
                    ctype val = stonum(arg, &ok);                              \
                    if (ok)                                                    \
                        structname.varname = val;                              \
                },                                                             \
            .print =                                                           \
                [](std::string args) {                                         \
                    (void)args;                                                \
                    printf(#structname "->" #varname " = " printf_fmt "\n",    \
                           structname.varname);                                \
                }};

#define REGISTER_BITFIELD(structname, varname, ctype, printf_fmt, stonum)      \
    assert(g_structs.count(#structname) &&                                     \
           "Trying to register var with unregistered struct: " #structname);   \
    g_structs[#structname].vars[#varname] =                                    \
        Var{.data = &structname,                                               \
            .size = sizeof(structname),                                        \
            .offset = 0,                                                       \
            .name = #varname,                                                  \
            .set =                                                             \
                [](std::string arg) {                                          \
                    bool ok;                                                   \
                    ctype val = stonum(arg, &ok);                              \
                    if (ok)                                                    \
                        structname.varname = val;                              \
                },                                                             \
            .print =                                                           \
                [](std::string args) {                                         \
                    (void)args;                                                \
                    printf(#structname "->" #varname " = " printf_fmt "\n",    \
                           structname.varname);                                \
                }};

#define REGISTER_ARR(structname, varname, length, ctype, printf_fmt, stonum)   \
    assert(g_structs.count(#structname) &&                                     \
           "Trying to register var with unregistered struct: " #structname);   \
    g_structs[#structname].vars[#varname] =                                    \
        Var{.data = &structname.varname,                                       \
            .size = sizeof(structname.varname),                                \
            .offset = (size_t)&structname.varname - (size_t)&structname,       \
            .name = #varname,                                                  \
            .set =                                                             \
                [](std::string arg) {                                          \
                    bool ok;                                                   \
                    auto args = split(arg);                                    \
                    for (size_t i = 0; i < args.size() && i < length; i++) {   \
                        ctype val = stonum(args[i], &ok);                      \
                        if (!ok)                                               \
                            return;                                            \
                        structname.varname[i] = val;                           \
                    }                                                          \
                },                                                             \
            .print =                                                           \
                [](std::string args) {                                         \
                    if (args.size() > 0) {                                     \
                        /*"TODO: get index*/                                   \
                    }                                                          \
                    for (int i = 0; i < length; i++) {                         \
                        printf(#structname "->" #varname "[%3d] = " printf_fmt \
                                           "\n",                               \
                               i, structname.varname[i]);                      \
                    }                                                          \
                }};

void print_var(std::string structname, std::string varname,
               std::string extra_args) {
    if (g_structs.count(structname) &&
        g_structs[structname].vars.count(varname)) {
        g_structs[structname].vars[varname].print(extra_args);
    } else {
        // not found
    }
}

#if 0
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
#endif

/**
 * EVERYTHING IN THIS FILE AFTER THIS LINE WILL BE OVERWRITTEN
 */

// pycstruct_shit
static struct _Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    long long d[4];
    float f;
} test;

void init_structs() {
    REGISTER_INTERNAL_STRUCT(_Test, test)
    REGISTER_VAR(test, a, int, "%9d", stol)
    REGISTER_ARR(test, d, 4, long long, "%20lld", stol)
    REGISTER_VAR(test, f, float, "%14.4e", stod)
    REGISTER_BITFIELD(test, b, unsigned int, "%08X", stoul_0x)
    REGISTER_BITFIELD(test, c, unsigned int, "%08X", stoul_0x)
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
