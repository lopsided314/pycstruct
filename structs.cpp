#include "structs.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <stdint.h>
#include <stdio.h>

namespace js = JStrings;

struct Var {
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

std::map<std::string, Struct> g_structs;

#define REGISTER_INTERNAL_STRUCT(structtype, sname)                            \
    g_structs[#sname] = Struct{.data = (uint8_t *)&sname,                      \
                               .size = sizeof(sname),                          \
                               .name = #sname,                                 \
                               .type = #structtype,                            \
                               .print =                                        \
                                   [](JStringList args) {                      \
                                       (void)args;                             \
                                       printf(#sname " print\n");              \
                                   },                                          \
                               .vars = {},                                     \
                               .working_addr = 0};

#define REGISTER_VAR(sname, vname, ctype, printf_fmt, stonum)                  \
    assert(g_structs.count(#sname) &&                                          \
           "Trying to register var with unregistered struct: " #sname);        \
    g_structs[#sname].vars[#vname] = Var{                                      \
        .data = (uint8_t *)&sname.vname,                                       \
        .size = sizeof(sname.vname),                                           \
        .offset = (ssize_t) & sname.vname - (ssize_t) & sname,                 \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](JStringList args) {                                             \
                bool ok;                                                       \
                ctype val = js::stonum(args[0], &ok);                          \
                if (ok)                                                        \
                    sname.vname = val;                                         \
            },                                                                 \
        .print =                                                               \
            [](JStringList args) {                                             \
                (void)args;                                                    \
                printf(#sname "->" #vname " = " printf_fmt "\n", sname.vname); \
            }};

#define REGISTER_BITFIELD(sname, vname, ctype, printf_fmt, stonum)             \
    assert(g_structs.count(#sname) &&                                          \
           "Trying to register var with unregistered struct: " #sname);        \
    g_structs[#sname].vars[#vname] = Var{                                      \
        .data = (uint8_t *)&sname,                                             \
        .size = sizeof(sname),                                                 \
        .offset = -1,                                                          \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](JStringList args) {                                             \
                bool ok;                                                       \
                ctype val = js::stonum(args[0], &ok);                          \
                if (ok)                                                        \
                    sname.vname = val;                                         \
            },                                                                 \
        .print =                                                               \
            [](JStringList args) {                                             \
                (void)args;                                                    \
                printf(#sname "->" #vname " = " printf_fmt "\n", sname.vname); \
            }};

#define REGISTER_ARR(sname, vname, length, ctype, printf_fmt, stonum)          \
    assert(g_structs.count(#sname) &&                                          \
           "Trying to register var with unregistered struct: " #sname);        \
    g_structs[#sname].vars[#vname] = Var{                                      \
        .data = (uint8_t *)&sname.vname,                                       \
        .size = sizeof(sname.vname),                                           \
        .offset = (ssize_t) & sname.vname - (ssize_t) & sname,                 \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](JStringList args) {                                             \
                bool ok;                                                       \
                for (size_t i = 0; i < args.size() && i < length; i++) {       \
                    ctype val = js::stonum(args[i], &ok);                      \
                    if (!ok)                                                   \
                        return;                                                \
                    sname.vname[i] = val;                                      \
                }                                                              \
            },                                                                 \
        .print =                                                               \
            [](JStringList args) {                                             \
                if (args.size() > 0) {                                         \
                    bool ok;                                                   \
                    int i = js::stol(args[0], &ok);                            \
                    if (!ok)                                                   \
                        printf(#sname "->" #vname "[%3d] = " printf_fmt "\n",  \
                               i, sname.vname[i]);                             \
                    return;                                                    \
                }                                                              \
                for (int i = 0; i < length; i++) {                             \
                    printf(#sname "->" #vname "[%3d] = " printf_fmt "\n", i,   \
                           sname.vname[i]);                                    \
                }                                                              \
            }};

void print_var(std::string sname, std::string vname, JStringList extra_args) {
    if (g_structs.count(sname) && g_structs[sname].vars.count(vname)) {
        g_structs[sname].vars[vname].print(extra_args);
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
    }
}

void set_var(std::string sname, std::string vname, JStringList args) {
    if (g_structs.count(sname) && g_structs[sname].vars.count(vname)) {
        g_structs[sname].vars[vname].set(args);
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
    }
}

VarData get_data(std::string sname, std::string vname) {
    if (g_structs.count(sname) && g_structs[sname].vars.count(vname)) {
        auto var = g_structs[sname].vars[vname];
        return VarData{
            .data = var.data + g_structs[sname].working_addr,
            .size = var.size,
        };
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
        return VarData{.data = nullptr, .size = 0};
    }
}

/**
 * EVERYTHING IN THIS FILE AFTER THIS LINE WILL BE OVERWRITTEN
 */

//pycstruct_shit
static struct _Test { int a ; unsigned int b:10 , :6 , c:12 , :4 ; long long d [ 4 ] ; float f ; } test;
static struct _Test2 { const char * str ; double dd ; } test2;


void init_structs()
{
    REGISTER_INTERNAL_STRUCT(_Test, test)
    REGISTER_VAR(test, a, int, "%9d", stol)
    REGISTER_ARR(test, d, 4, long long, "%20lld", stol)
    REGISTER_VAR(test, f, float, "%14.4e", stod)
    REGISTER_BITFIELD(test, b, unsigned int, "%08X", stoul_0x)
    REGISTER_BITFIELD(test, c, unsigned int, "%08X", stoul_0x)    REGISTER_INTERNAL_STRUCT(_Test2, test2)
    REGISTER_VAR(test2, dd, double, "%14.4e", stod)
}

