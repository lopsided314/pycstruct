#include "structs.h"
#include "jstrings.hpp"

#include <cassert>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <vector>

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
        .sizeof_ctype = sizeof(sname.vname),                                   \
        .offset = (ssize_t) & sname.vname - (ssize_t) & sname,                 \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](JStringList args) {                                             \
                ctype val = js::stonum(args[0], &g_err);                       \
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
        .sizeof_ctype = sizeof(ctype),                                         \
        .offset = 0,                                                           \
        .name = #vname,                                                        \
        .set =                                                                 \
            [](const JStringList &args) {                                      \
                ctype val = js::stonum(args[0], &g_err);                       \
                if (g_err.length() > 0)                                        \
                    std::cout << "Failed " << g_err << "\n";                   \
                else                                                           \
                    sname.vname = val;                                         \
            },                                                                 \
        .print =                                                               \
            [](const JStringList &args) {                                      \
                (void)args;                                                    \
                printf(#sname "->" #vname " = " printf_fmt "\n", sname.vname); \
            }};

#define REGISTER_ARR(sname, vname, length, ctype, printf_fmt, stonum)          \
    assert(g_structs.count(#sname) &&                                          \
           "Trying to register var with unregistered struct: " #sname);        \
    g_structs[#sname].vars[#vname] =                                           \
        Var{.type = VarType::Array,                                            \
            .data = (uint8_t *)&sname.vname,                                   \
            .size = sizeof(sname.vname),                                       \
            .sizeof_ctype = sizeof(ctype),                                     \
            .offset = (ssize_t) & sname.vname - (ssize_t) & sname,             \
            .name = #vname,                                                    \
            .set =                                                             \
                [](const JStringList &args) {                                  \
                    auto indices = get_array_indices(args[1], length);         \
                    for (size_t iIdx = 0, iArg = 0;                            \
                         iIdx < indices.size() && iArg < args.size();          \
                         iIdx++, iArg++) {                                     \
                    }                                                          \
                    for (int i : indices) {                                    \
                    }                                                          \
                    for (size_t i = 2; i < args.size() && i < length; i++) {   \
                        ctype val = js::stonum(args[i], &g_err);               \
                        if (g_err.size() > 0) {                                \
                            std::cout << "Failed " << g_err << "\n";           \
                            return;                                            \
                        } else {                                               \
                            sname.vname[i] = val;                              \
                        }                                                      \
                    }                                                          \
                },                                                             \
            .print =                                                           \
                [](const JStringList &args) {                                  \
                    auto indices = get_array_indices(args[1], length);         \
                    for (int i : indices) {                                    \
                        printf(#sname "->" #vname "[%3d] = " printf_fmt "\n",  \
                               i, sname.vname[i]);                             \
                    }                                                          \
                }};

StructFind get_struct(std::string svname) {

    StructFind ret{.s = nullptr, .v = nullptr};
    auto sv_name_split = js::split(svname, "->", js::SkipEmpty | js::TrimAll);

    if (sv_name_split.size() != 2) {
        std::cout << "Invalid struct name\n";
        return ret;
    }

    std::string sname = sv_name_split[0];
    std::string vname = sv_name_split[1];

    if (js::contains_all(vname, "[]")) {
        js::slice(&vname, 0, vname.find('['));
    }
    if (g_structs.count(sname)) {
        if (g_structs[sname].vars.count(vname)) {
            ret.s = &g_structs[sname];
            ret.v = &g_structs[sname].vars[vname];
        } else {
            ret.s = &g_structs[sname];
        }
    } else {
        std::cout << "struct " << sname << "->" << vname << " not found'\n";
    }
    return ret;
}

std::vector<int> get_array_indices(std::string vname, int index_limit) {
    std::vector<int> indices;
    std::string brace_contents = "";

    if (js::contains_all(vname, "[]")) {
        brace_contents = js::slice(vname, vname.find('[') + 1, -1);
        if (brace_contents.length() == 0) {
            std::cout << "Invalid array indices\n";
            return {};
        }
    }

    if (brace_contents.length() == 0) {
        for (int i = 0; i < index_limit; i++) {
            indices.push_back(i);
        }
        return indices;
    }

    const int INDEX_ERR = js::end;
    auto index_stol = [index_limit](std::string s) -> int {
        if (s.length() == 0)
            return 0;

        int i = js::stol(s, &g_err);
        if (i < 0)
            i = index_limit - i;
        if (g_err.length() > 0) {
            std::cout << "Invalid array index: " << g_err << "\n";
            return INDEX_ERR;
        }
        if (i < 0 || i >= index_limit) {
            std::cout << "Invalid array index: " << s << "\n";
            return INDEX_ERR;
        }
        return i;
    };

    if (js::contains(brace_contents, ":")) {

        auto brace_split = js::split(brace_contents, ":", js::TrimAll);
        if (brace_split.size() != 2) {
            std::cout << "Invalid array slice\n";
            return {};
        }

        int start = index_stol(brace_split[0]);
        if (start == INDEX_ERR)
            return {};

        int stop = index_stol(brace_split[1]);
        if (stop == INDEX_ERR)
            return {};
        else if (stop == 0)
            stop = index_limit - 1;

        for (int i = start; i < stop; i++) {
            indices.push_back(i);
        }

    } else {
        auto brace_split =
            js::split(brace_contents, ",", js::TrimAll | js::SkipEmpty);

        for (const std::string &s : brace_split) {
            if (s.length() == 0)
                continue;

            int i = index_stol(s);
            if (i == INDEX_ERR)
                return {};

            indices.push_back(i);
        }
    }
    return indices;
}

StructParseOutput parse_struct_addr_cmd(const StructFind &sv,
                                        const JStringList &args) {

    StructParseOutput ret{};

    if (args.size() != 3 || sv.v) {
        std::cout << "Invalid args for setting struct address\n";
        return ret;
    } else {
        size_t new_addr = js::stoul_0x(args[2], &g_err);
        if (g_err.length() > 0) {
            std::cout << "set struct addr: " << g_err << "\n";
        } else {
            sv.s->working_addr = new_addr;
            ret.op = PASS;
        }
        return ret;
    }

    return ret;
}

StructParseOutput parse_struct_read_cmd(const StructFind &sv,
                                        const JStringList &args) {
    StructParseOutput ret{};

    if (args.size() != 2) {
        std::cout << "Invalid args for reading struct\n";
        return ret;
    }

    ret.op = READ;
    ret.s = sv.s;
    ret.v = sv.v;
    ret.data = sv.v->data + sv.v->offset;
    ret.size = sv.v->size;
    return ret;
}

StructParseOutput parse_struct_write_cmd(const StructFind &sv,
                                         const JStringList &args) {
    StructParseOutput ret{};

    if (args.size() < 3) {
        std::cout << "Invalid args for writing struct\n";
        return ret;
    }

    ret.s = sv.s;
    ret.v = sv.v;
    ret.data = sv.v->data + sv.v->offset;
    ret.size = sv.v->size;

    if (sv.v->type == Std) {
        sv.v->set({args[2]});
        ret.op = WRITE;
    } else if (sv.v->type == Array) {
        sv.v->set(args);
        ret.op = WRITE;
    } else if (sv.v->type == BField) {
        ret.op = WRITE_BITFIELD;
    } else {
        std::cout << "how did you get here?\n";
    }
    return ret;
}

StructParseOutput parse_struct_input(const JStringList &args) {

    if (args.size() < 2) {
        std::cout << "Invalid args for struct operation\n";
        return {};
    }

    auto sv = get_struct(args[1]);

    if (!sv.s) {
        std::cout << "Couldn't find struct \"" << args[1] << "\"\n";
        return {};
    }

    bool addr_cmd = args[0] == "com" || args[0] == "mv";
    bool write_cmd = args[0] == "co";
    bool read_cmd = args[0] == "ci";

    if (addr_cmd) {
        return parse_struct_addr_cmd(sv, args);
    }

    if (!sv.v) {
        std::cout << "Couldn't find \"" << args[1] << "\"\n";
        return {};
    }

    if (read_cmd) {
        return parse_struct_read_cmd(sv, args);
    }

    if (write_cmd) {
        return parse_struct_write_cmd(sv, args);
    }

    return {};
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
