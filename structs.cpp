#include "structs.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "string_functions.hpp"

struct Var {
    void *data;
    size_t size;
    ssize_t offset;
    std::string name;
    std::function<void(std::string)> set;
    std::function<void(std::string)> print;
};

struct Struct {
    void *data;
    size_t size;
    std::string name;
    std::string type;
    std::function<void(std::string)> print;
    std::map<std::string, Var> vars;
};

std::map<std::string, Struct> g_structs;
std::vector<std::string> split(std::string) { return {}; }
void replace(std::string *) {}

const std::map<std::string, std::string> printf_fmts{{"i32", "%10d"}};

#define REGISTER_INTERNAL_STRUCT(structtype, sname)                            \
    g_structs[#sname] = Struct{.data = &sname,                                 \
                               .size = sizeof(sname),                          \
                               .name = #sname,                                 \
                               .type = #structtype,                            \
                               .print =                                        \
                                   [](std::string args) {                      \
                                       (void)args;                             \
                                       printf(#sname " print\n");              \
                                   },                                          \
                               .vars = {}};

#define REGISTER_VAR(sname, vname, ctype, printf_fmt, stonum)                  \
    assert(g_structs.count(#sname) &&                                          \
           "Trying to register var with unregistered struct: " #sname);        \
    g_structs[#sname].vars[#vname] = Var{                                      \
        .data = &sname.vname,                                                  \
        .size = sizeof(sname.vname),                                           \
        .offset = (ssize_t) & sname.vname - (ssize_t) & sname,                 \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](std::string arg) {                                              \
                bool ok;                                                       \
                ctype val = stonum(arg, &ok);                                  \
                if (ok)                                                        \
                    sname.vname = val;                                         \
            },                                                                 \
        .print =                                                               \
            [](std::string args) {                                             \
                (void)args;                                                    \
                printf(#sname "->" #vname " = " printf_fmt "\n", sname.vname); \
            }};

#define REGISTER_BITFIELD(sname, vname, ctype, printf_fmt, stonum)             \
    assert(g_structs.count(#sname) &&                                          \
           "Trying to register var with unregistered struct: " #sname);        \
    g_structs[#sname].vars[#vname] = Var{                                      \
        .data = &sname,                                                        \
        .size = sizeof(sname),                                                 \
        .offset = -1,                                                          \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](std::string arg) {                                              \
                bool ok;                                                       \
                ctype val = stonum(arg, &ok);                                  \
                if (ok)                                                        \
                    sname.vname = val;                                         \
            },                                                                 \
        .print =                                                               \
            [](std::string args) {                                             \
                (void)args;                                                    \
                printf(#sname "->" #vname " = " printf_fmt "\n", sname.vname); \
            }};

#define REGISTER_ARR(sname, vname, length, ctype, printf_fmt, stonum)          \
    assert(g_structs.count(#sname) &&                                          \
           "Trying to register var with unregistered struct: " #sname);        \
    g_structs[#sname].vars[#vname] =                                           \
        Var{.data = &sname.vname,                                              \
            .size = sizeof(sname.vname),                                       \
            .offset = (ssize_t) & sname.vname - (ssize_t) & sname,             \
            .name = #vname,                                                    \
            .set =                                                             \
                [](std::string arg) {                                          \
                    (void)arg;                                                 \
                    bool ok;                                                   \
                    auto args = std::vector<std::string>{"20", "30"};          \
                    for (size_t i = 0; i < args.size() && i < length; i++) {   \
                        ctype val = stonum(args[i], &ok);                      \
                        if (!ok)                                               \
                            return;                                            \
                        sname.vname[i] = val;                                  \
                    }                                                          \
                },                                                             \
            .print =                                                           \
                [](std::string args) {                                         \
                    if (args.size() > 0) {                                     \
                        bool ok;                                               \
                        int i = stol(args, &ok);                               \
                        if (!ok)                                               \
                            return;                                            \
                        printf(#sname "->" #vname "[%3d] = " printf_fmt "\n",  \
                               i, sname.vname[i]);                             \
                        return;                                                \
                    }                                                          \
                    for (int i = 0; i < length; i++) {                         \
                        printf(#sname "->" #vname "[%3d] = " printf_fmt "\n",  \
                               i, sname.vname[i]);                             \
                    }                                                          \
                }};

void print_var(std::string sname, std::string vname, std::string extra_args) {
    if (g_structs.count(sname) && g_structs[sname].vars.count(vname)) {
        g_structs[sname].vars[vname].print(extra_args);
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
    }
}

void set_var(std::string sname, std::string vname, std::string args) {
    if (g_structs.count(sname) && g_structs[sname].vars.count(vname)) {
        g_structs[sname].vars[vname].set(args);
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
    }
}

VarData get_data(std::string sname, std::string vname) {
    if (g_structs.count(sname) && g_structs[sname].vars.count(vname)) {
        return VarData{
            .data = g_structs[sname].vars[vname].data,
            .size = g_structs[sname].vars[vname].size,
        };
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
        return VarData{
            .data = nullptr,
            .size = 0
        };
    }
}


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
static struct _Test2 {
    const char *str;
    double dd;
} test2;

void init_structs() {
    REGISTER_INTERNAL_STRUCT(_Test, test)
    REGISTER_VAR(test, a, int, "%9d", stol)
    REGISTER_ARR(test, d, 4, long long, "%20lld", stol)
    REGISTER_VAR(test, f, float, "%14.4e", stod)
    REGISTER_BITFIELD(test, b, unsigned int, "%08X", stoul_0x)
    REGISTER_BITFIELD(test, c, unsigned int, "%08X", stoul_0x)
    REGISTER_INTERNAL_STRUCT(_Test2, test2)
        REGISTER_VAR(test2, dd, double, "%14.4e", stod)
}
