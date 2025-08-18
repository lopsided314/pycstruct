#include "structs.hpp"
#include "utils/jstrings/jstrings.hpp"

#include <cassert>
#include <iostream>
#include <stdio.h>
#include <vector>

static std::string g_err;

namespace js = JStrings;
std::map<std::string, Struct> g_structs;

#define REGISTER_INTERNAL_STRUCT(structtype, sname)                                                \
    g_structs[#sname] = Struct{};                                                                  \
    g_structs[#sname].data = (uint8_t *)&sname;                                                    \
    g_structs[#sname].size = sizeof(sname);                                                        \
    g_structs[#sname].name = #sname;                                                               \
    g_structs[#sname].type = #structtype;                                                          \
    g_structs[#sname].print = [](const JStringList &args) {                                        \
        for (const auto &var : g_structs[#sname].vars) {                                           \
            var.second.print(args);                                                                \
        }                                                                                          \
        std::cout << "\n";                                                                         \
    };

#define REGISTER_VAR(sname, vname, ctype, printf_fmt, stonum)                                      \
    assert(g_structs.count(#sname) && "Trying to register var with unregistered struct: " #sname); \
    g_structs[#sname].vars[#vname] = Var{};                                                        \
    g_structs[#sname].vars[#vname].type = VarType::Std;                                            \
    g_structs[#sname].vars[#vname].data = (uint8_t *)&sname.vname;                                 \
    g_structs[#sname].vars[#vname].size = sizeof(sname.vname);                                     \
    g_structs[#sname].vars[#vname].sizeof_ctype = sizeof(sname.vname);                             \
    g_structs[#sname].vars[#vname].offset = (size_t)&sname.vname - (size_t)&sname;                 \
    g_structs[#sname].vars[#vname].name = #vname;                                                  \
    g_structs[#sname].vars[#vname].set = [](const JStringList &args) {                             \
        ctype val = cout_##stonum(args[2]);                                                        \
        if (g_err.length() == 0)                                                                   \
            sname.vname = val;                                                                     \
    };                                                                                             \
    g_structs[#sname].vars[#vname].print = [](const JStringList &args) {                           \
        (void)args;                                                                                \
        printf(#sname "->" #vname " = " printf_fmt "\n", sname.vname);                             \
    };

#define REGISTER_BITFIELD(sname, vname, ctype, printf_fmt, stonum)                                 \
    assert(g_structs.count(#sname) && "Trying to register var with unregistered struct: " #sname); \
    g_structs[#sname].vars[#vname] = Var{};                                                        \
    g_structs[#sname].vars[#vname].type = VarType::BField;                                         \
    g_structs[#sname].vars[#vname].data = (uint8_t *)&sname;                                       \
    g_structs[#sname].vars[#vname].size = sizeof(sname);                                           \
    g_structs[#sname].vars[#vname].sizeof_ctype = sizeof(ctype);                                   \
    g_structs[#sname].vars[#vname].offset = 0;                                                     \
    g_structs[#sname].vars[#vname].name = #vname;                                                  \
    g_structs[#sname].vars[#vname].set = [](const JStringList &args) {                             \
        ctype val = cout_##stonum(args[2]);                                                        \
        if (g_err.length() == 0)                                                                   \
            sname.vname = val;                                                                     \
    };                                                                                             \
    g_structs[#sname].vars[#vname].print = [](const JStringList &args) {                           \
        (void)args;                                                                                \
        printf(#sname "->" #vname " = " printf_fmt "\n", sname.vname);                             \
    };

#define REGISTER_ARR(sname, vname, length, ctype, printf_fmt, stonum)                              \
    assert(g_structs.count(#sname) && "Trying to register var with unregistered struct: " #sname); \
    g_structs[#sname].vars[#vname] = Var{};                                                        \
    g_structs[#sname].vars[#vname].type = VarType::Array;                                          \
    g_structs[#sname].vars[#vname].data = (uint8_t *)&sname;                                       \
    g_structs[#sname].vars[#vname].size = sizeof(sname.vname);                                     \
    g_structs[#sname].vars[#vname].sizeof_ctype = sizeof(ctype);                                   \
    g_structs[#sname].vars[#vname].offset = (size_t)&sname.vname[0] - (size_t)&sname;              \
    g_structs[#sname].vars[#vname].name = #vname;                                                  \
    g_structs[#sname].vars[#vname].set = [](const JStringList &args) {                             \
        std::vector<int> indices = get_array_slice_indices(args[1], length);                       \
        for (size_t i = 0; i < indices.size() && i < args.size() - 2; i++) {                       \
            ctype val = cout_##stonum(args[i + 2]);                                                \
            if (g_err.size() == 0)                                                                 \
                sname.vname[indices[i]] = val;                                                     \
            else                                                                                   \
                return;                                                                            \
        }                                                                                          \
    };                                                                                             \
    g_structs[#sname].vars[#vname].print = [](const JStringList &args) {                           \
        auto indices = get_array_slice_indices(args[1], length);                                   \
        for (int i : indices) {                                                                    \
            printf(#sname "->" #vname "[%3d] = " printf_fmt "\n", i, sname.vname[i]);              \
        }                                                                                          \
    };

/*================================================================================*/

static unsigned long cout_stoul_0x(std::string str) {
    unsigned long val = js::stoul_0x(str, &g_err);
    if (g_err.length() > 0) {
        std::cout << "Failed " << g_err << "\n";
    }
    return val;
}
static long cout_stol(std::string str) {
    long val = js::stol(str, &g_err);
    if (g_err.length() > 0) {
        std::cout << "Failed " << g_err << "\n";
    }
    return val;
}
static double cout_stod(std::string str) {
    double val = js::stod(str, &g_err);
    if (g_err.length() > 0) {
        std::cout << "Failed " << g_err << "\n";
    }
    return val;
}

/*================================================================================*/

static std::vector<int> get_array_slice_indices(std::string vname, int index_limit) {
    std::vector<int> indices;
    std::string brace_contents = "";

    if (js::contains_all(vname, "[]")) {
        brace_contents = js::slice(vname, vname.find('[') + 1, vname.find(']'));
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
            i = index_limit + i;
        if (g_err.length() > 0 || i < 0 || i > index_limit) {
            std::cout << "Invalid array index: " << s << "\n";
            return INDEX_ERR;
        }
        return i;
    };

    // python style slicing
    if (js::contains(brace_contents, ":")) {

        auto brace_split = js::split(brace_contents, ":", js::TrimAll);
        if (brace_split.size() != 2) {
            std::cout << "Invalid array slice\n";
            return {};
        }

        int start = index_stol(brace_split[0]);
        int stop = index_stol(brace_split[1]);
        if (start == INDEX_ERR || stop == INDEX_ERR)
            return {};

        if (stop == 0)
            stop = index_limit;

        for (int i = start; i < stop; i++) {
            indices.push_back(i);
        }

    } else {
        // csv
        auto brace_split = js::split(brace_contents, ",", js::TrimAll | js::SkipEmpty);
        for (const std::string &s : brace_split) {

            int i = index_stol(s);
            if (i == index_limit) {
                std::cout << "Invalid array slice\n";
                return {};
            } else if (i == INDEX_ERR) {
                return {};
            }

            indices.push_back(i);
        }
    }
    return indices;
}

/*================================================================================*/

StructFind get_struct(std::string svname) {

    StructFind ret{};
    auto strs = js::split(svname, "->", js::TrimAll);

    if (strs.size() != 2) {
        std::cout << "Invalid struct name\n";
        return ret;
    }

    std::string sname = strs[0];
    std::string vname = strs[1];

    if (js::contains_all(vname, "[]")) {
        js::slice(&vname, 0, vname.find('['));
    }
    if (g_structs.count(sname)) {
        ret.s = &g_structs[sname];
        if (g_structs[sname].vars.count(vname)) {
            ret.v = &g_structs[sname].vars[vname];
        }
    } else {
        std::cout << "struct " << svname << " not found'\n";
    }
    return ret;
}

/*================================================================================*/

StructParseOutput parse_struct_write_cmd(const StructFind &sv, const JStringList &args) {
    StructParseOutput ret{};

    if (args.size() < 3) {
        std::cout << "Invalid args for writing struct\n";
        return ret;
    }

    ret.s = sv.s;
    ret.v = sv.v;
    ret.data = sv.v->data + sv.v->offset;
    ret.size = sv.v->size;
    ret.offset = sv.v->offset + sv.s->working_addr;

    switch (sv.v->type) {
    case Std:

    case Array:
        sv.v->set(args);
        ret.op = WRITE;
        break;
    case BField:
        ret.op = WRITE_BITFIELD;
        break;
    }
    return ret;
}

/*================================================================================*/

StructParseOutput parse_struct_addr_cmd(const StructFind &sv, const JStringList &args) {

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
            ret.s = sv.s;
            ret.v = sv.v;
        }
        return ret;
    }

    return ret;
}

/*================================================================================*/

StructParseOutput parse_struct_read_cmd(const StructFind &sv, const JStringList &args) {
    StructParseOutput ret{};

    if (args.size() != 2) {
        std::cout << "Invalid args for reading struct\n";
        return ret;
    }

    ret.s = sv.s;
    if (sv.v) {
        ret.op = READ_VAR;
        ret.v = sv.v;
        ret.data = sv.v->data + sv.v->offset;
        ret.size = sv.v->size;
        ret.offset = sv.v->offset + sv.s->working_addr;
    } else {
        ret.op = READ_STRUCT;
        ret.data = sv.s->data;
        ret.size = sv.s->size;
        ret.offset = sv.s->working_addr;
    }
    return ret;
}

/*================================================================================*/

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

    if (read_cmd) {
        return parse_struct_read_cmd(sv, args);
    }

    if (!sv.v) {
        std::cout << "Couldn't find \"" << args[1] << "\"\n";
        return {};
    }

    if (write_cmd) {
        return parse_struct_write_cmd(sv, args);
    }

    return {};
}

/*================================================================================*/

/**
 * EVERYTHING IN THIS FILE AFTER THIS LINE WILL BE OVERWRITTEN
 */

// pycstruct_shit
static struct _Test2 {
    const char *str;
    double dd;
} test2;
static struct _Test {
    int a;
    unsigned int b : 10, : 6, c : 12, : 4;
    float d[4];
    float f;
} test;

void init_structs() {
    REGISTER_INTERNAL_STRUCT(_Test2, test2);
    REGISTER_VAR(test2, dd, double, "%14.4e", stod);

    REGISTER_INTERNAL_STRUCT(_Test, test);
    REGISTER_VAR(test, a, int, "%9d", stol);
    REGISTER_ARR(test, d, 4, float, "%14.4e", stod);
    REGISTER_VAR(test, f, float, "%14.4e", stod);
    REGISTER_BITFIELD(test, b, unsigned int, "%8X", stoul_0x);
    REGISTER_BITFIELD(test, c, unsigned int, "%8X", stoul_0x);
}
