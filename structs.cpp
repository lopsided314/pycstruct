#include "structs.h"
#include "jstrings.hpp"

#include <cassert>
#include <iostream>
#include <stdint.h>
#include <stdio.h>

static std::string g_err;

namespace js = JStrings;
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
        .type = VarType::Std,                                                  \
        .data = (uint8_t *)&sname.vname,                                       \
        .size = sizeof(sname.vname),                                           \
        .offset = (ssize_t) & sname.vname - (ssize_t) & sname,                 \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](JStringList args) {                                             \
                ctype val = js::stonum(args[0], nullptr, &g_err);              \
                if (g_err.length() > 0)                                        \
                    std::cout << "Failed " << g_err << "\n";                   \
                else                                                           \
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
        .type = VarType::BField,                                               \
        .data = (uint8_t *)&sname,                                             \
        .size = sizeof(sname),                                                 \
        .offset = 0,                                                           \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](JStringList args) {                                             \
                ctype val = js::stonum(args[0], nullptr, &g_err);              \
                if (g_err.length() > 0)                                        \
                    std::cout << "Failed " << g_err << "\n";                   \
                else                                                           \
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
        .type = VarType::Array,                                                \
        .data = (uint8_t *)&sname.vname,                                       \
        .size = sizeof(sname.vname),                                           \
        .offset = (ssize_t) & sname.vname - (ssize_t) & sname,                 \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](JStringList args) {                                             \
                for (size_t i = 0; i < args.size() && i < length; i++) {       \
                    ctype val = js::stonum(args[i], nullptr, &g_err);          \
                    if (g_err.size() > 0) {                                    \
                        std::cout << "Failed " << g_err << "\n";               \
                        return;                                                \
                    } else {                                                   \
                        sname.vname[i] = val;                                  \
                    }                                                          \
                }                                                              \
            },                                                                 \
        .print =                                                               \
            [](JStringList args) {                                             \
                if (args.size() > 0) {                                         \
                    int i = js::stol(args[0], nullptr, &g_err);                \
                    if (g_err.size() > 0)                                      \
                        std::cout << "Failed " << g_err << "\n";               \
                    else                                                       \
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
            .type = var.type,
            .data = var.data + g_structs[sname].working_addr,
            .size = var.size,
        };
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
        return VarData{.type = VarType::Std, .data = nullptr, .size = 0};
    }
}

StructFind get_struct(std::string sname, std::string vname) {

    if (js::contains_all(vname, "[]")) {
        js::slice(&vname, 0, vname.find('['));
    }
    if (g_structs.count(sname)) {
        if (g_structs[sname].vars.count(vname)) {
            auto var = g_structs[sname].vars[vname];
            return StructFind{
                .s = &g_structs[sname],
                .v = &g_structs[sname].vars[vname],
            };
        } else {
            return StructFind{
                .s = &g_structs[sname],
                .v = nullptr,
            };
        }
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
    }
    return {};
}

void parse_struct_input(JStringList args) {
    if (args.size() < 2) {
        std::cout << "Invalid args for struct operation\n";
        return;
    }

    auto sv_name_split = js::split(args[1], "->", js::SkipEmpty | js::TrimAll);

    if (sv_name_split.size() != 2) {
        std::cout << "Invalid struct name\n";
        return;
    }

    auto sv = get_struct(sv_name_split[0], sv_name_split[1]);

    if (!sv.s) {
        std::cout << "Couldn't find struct \"" << sv_name_split[0] << "\"\n";
        return;
    }

    bool addr_cmd = args[0] == "com";
    bool write_cmd = args[0] == "co";
    bool read_cmd = args[0] == "ci";

    if ((addr_cmd && args.size() != 3) || (addr_cmd && sv.v)) {
        std::cout << "Invalid args for setting struct address\n";
        return;
    } else if (addr_cmd) {
        size_t new_addr = js::stoul_0x(args[2], &g_err);
        if (g_err.length() > 0) {
            std::cout << "set struct addr: " << g_err << "\n";
        } else {
            sv.s->working_addr = new_addr;
        }
        return;
    }

    if (!sv.v) {
        std::cout << "Couldn't find \"" << args[1] << "\"\n";
        return;
    }

    if (read_cmd && args.size() != 2) {
        std::cout << "Invalid args for reading struct member\n";
        return;
    } else if (read_cmd) {
        if (sv.v->type == VarType::Array &&
            js::contains_all(sv_name_split[1], "[]")) {
            std::string arr_index =
                js::slice(sv_name_split[1], sv_name_split[1].find('['), -1);
            sv.v->print({arr_index});
        } else {
            sv.v->print({});
        }
        return;
    }

    if (write_cmd && args.size() < 3) {
        std::cout << "Invalid args for reading struct member\n";
        return;
    } else if (write_cmd) {
        if (sv.v->type == VarType::BField) {

        }
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
