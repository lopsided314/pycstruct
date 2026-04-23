/******************************************************************************
 *
 * The way this thing is supposed to work is the python script (pycstruct2.py)
 * will parse all the source files to find instances where someone has asked
 * to register a struct. The python is then responsible for finding and parsing
 * the source code and generates the init function at the end of this file which
 * contains all the macros that create the wrappers around the structs and their
 * variables allowing their data to be accessed by name.
 *
 *
 * The python script will add new static instances of all the registered
 * structs. These static instances are the memory that the struct write/read
 * functions use as the source/destination of data.
 *
 * The g_structs map is a mapping of the name of the struct, i.e. the token
 * with which the user will access this memory from the cli prompt, to a struct
 * wrapper type which contains metadata for the struct. Every registered struct
 * gets an entry into this map.
 *
 * The REGISTER_INTERNAL_STRUCT macro will create a 'Struct' object and
 * initialize its members including metadata, a pointer to the static
 * instance memory for that struct type, its name, and a variable container.
 *
 * The variable registration macros add a 'Var' object to the parent structs
 * variable container and initialize the metadata and data pointers as well
 * as the getters/setters (so to speak). Because the lambdas are declared
 * in macros, all the set and print functions use '=' instead of needing
 * to copy data around through pointers. This is how the program deals with
 * bitfields. Since bitfields don't have an address, they cannot have data
 * copied to/from them via pointer and must be set by value.
 *
 * The 'set' and 'print' functions have no interaction with memory outside
 * this file. They only operate on the memory in the local struct instances.
 *
 * For read/write operations, the local instances are essentially a translator
 * for the data, the data transfer is done somewhere else. For 'reading' from
 * a struct that is appears to be at some offset, what is really happening is
 * the data at that address is being copied into the local struct instance
 * and the print function displays what is in the local instance memory. Same
 * with writing, the value input from the user is parsed and put into local
 * memory and then copied out from the local instance.
 *
 * Since bitfields don't have an address the data pointer returned by the
 * read/write request is the base address of the whole struct. Arrays will
 * also read/write the full size regardless of which values are being messed
 * with.
 *
 ******************************************************************************/

#include "structs.hpp"

#include <cassert>
#include <climits>
#include <ctime>
#include <iostream>
#include <map>
#include <stdexcept>
#include <sys/stat.h>
#include <vector>

#include "utils/datetime/datetime.hpp"
#include "utils/jstrings/jstrings.hpp"

static std::string g_err;

namespace js = JStrings;

// The print here allows for printing all partial variable name matches. Makes things
// like name->plpl* possible.
#define REGISTER_INTERNAL_STRUCT(structtype, sname, src_path, raw_src)                             \
    g_structs.emplace(#sname, Struct{});                                                           \
    g_structs.at(#sname).data = (uint8_t *)&sname;                                                 \
    g_structs.at(#sname).size = sizeof(sname);                                                     \
    g_structs.at(#sname).name = #sname;                                                            \
    g_structs.at(#sname).type = #structtype;                                                       \
    g_structs.at(#sname).src_filepath = src_path;                                                  \
    g_structs.at(#sname).src_definition = raw_src;                                                 \
    g_structs.at(#sname).print = [](const JStringList &args) {                                     \
        const std::string partial_name = js::strip(args[1], "*");                                  \
        for (const auto &var : g_structs.at(#sname).vars) {                                        \
            if (js::contains(#sname "->" + var.first, partial_name)) {                             \
                var.second.print(args);                                                            \
            }                                                                                      \
        }                                                                                          \
        std::cout << "\n";                                                                         \
    };

#define REGISTER_VAR(sname, vname, ctype, printf_fmt, stonum)                                      \
    assert(g_structs.count(#sname) && "Trying to register var with unregistered struct: " #sname); \
    g_structs.at(#sname).vars.emplace(#vname, Var{});                                              \
    g_structs.at(#sname).vars.at(#vname).type = Var::VarType::Std;                                 \
    g_structs.at(#sname).vars.at(#vname).data = (uint8_t *)&sname.vname;                           \
    g_structs.at(#sname).vars.at(#vname).size = sizeof(sname.vname);                               \
    g_structs.at(#sname).vars.at(#vname).sizeof_ctype = sizeof(sname.vname);                       \
    g_structs.at(#sname).vars.at(#vname).offset = (size_t)&sname.vname - (size_t)&sname;           \
    g_structs.at(#sname).vars.at(#vname).name = #vname;                                            \
    g_structs.at(#sname).vars.at(#vname).set = [](const JStringList &args) {                       \
        ctype val = cout_##stonum(args[2]);                                                        \
        if (g_err.length() == 0) {                                                                 \
            sname.vname = val;                                                                     \
        }                                                                                          \
    };                                                                                             \
    g_structs.at(#sname).vars.at(#vname).print = [](const JStringList &args) {                     \
        (void)args;                                                                                \
        std::cout << js::fmt(#sname "->" #vname " = " printf_fmt "\n", sname.vname);               \
    };

#define REGISTER_CHAR_ARR(sname, vname)                                                            \
    assert(g_structs.count(#sname) && "Trying to register var with unregistered struct: " #sname); \
    g_structs.at(#sname).vars.emplace(#vname, Var{});                                              \
    g_structs.at(#sname).vars.at(#vname).type = Var::VarType::Std;                                 \
    g_structs.at(#sname).vars.at(#vname).data = (uint8_t *)&sname.vname;                           \
    g_structs.at(#sname).vars.at(#vname).size = sizeof(sname.vname);                               \
    g_structs.at(#sname).vars.at(#vname).sizeof_ctype = sizeof(char);                              \
    g_structs.at(#sname).vars.at(#vname).offset = (size_t)&sname.vname - (size_t)&sname;           \
    g_structs.at(#sname).vars.at(#vname).name = #vname;                                            \
    g_structs.at(#sname).vars.at(#vname).set = [](const JStringList &args) {                       \
        std::string s = args[2];                                                                   \
        for (size_t i = 3; i < args.size(); i++) {                                                 \
            s += " " + args[i];                                                                    \
        }                                                                                          \
        strncpy(sname.vname, s.c_str(), sizeof(sname.vname) - 1);                                  \
        sname.vname[sizeof(sname.vname) - 1] = '\0';                                               \
    };                                                                                             \
    g_structs.at(#sname).vars.at(#vname).print = [](const JStringList &args) {                     \
        (void)args;                                                                                \
        sname.vname[sizeof(sname.vname) - 1] = '\0';                                               \
        std::cout << js::fmt(#sname "->" #vname " = \"%s\"", sname.vname);                         \
    };

#define REGISTER_BITFIELD(sname, vname, ctype, printf_fmt, stonum)                                 \
    assert(g_structs.count(#sname) && "Trying to register var with unregistered struct: " #sname); \
    g_structs.at(#sname).vars.emplace(#vname, Var{});                                              \
    g_structs.at(#sname).vars.at(#vname).type = Var::VarType::BField;                              \
    g_structs.at(#sname).vars.at(#vname).data = (uint8_t *)&sname;                                 \
    g_structs.at(#sname).vars.at(#vname).size = sizeof(sname);                                     \
    g_structs.at(#sname).vars.at(#vname).sizeof_ctype = sizeof(ctype);                             \
    g_structs.at(#sname).vars.at(#vname).offset = 0;                                               \
    g_structs.at(#sname).vars.at(#vname).name = #vname;                                            \
    g_structs.at(#sname).vars.at(#vname).set = [](const JStringList &args) {                       \
        ctype val = cout_##stonum(args[2]);                                                        \
        if (g_err.length() == 0) {                                                                 \
            sname.vname = val;                                                                     \
        }                                                                                          \
    };                                                                                             \
    g_structs.at(#sname).vars.at(#vname).print = [](const JStringList &args) {                     \
        (void)args;                                                                                \
        std::cout << js::fmt(#sname "->" #vname " = " printf_fmt "\n", sname.vname);               \
    };

#define REGISTER_ARR(sname, vname, length, ctype, printf_fmt, stonum)                              \
    assert(g_structs.count(#sname) && "Trying to register var with unregistered struct: " #sname); \
    g_structs.at(#sname).vars.emplace(#vname, Var{});                                              \
    g_structs.at(#sname).vars.at(#vname).type = Var::VarType::Array;                               \
    g_structs.at(#sname).vars.at(#vname).data = (uint8_t *)&sname.vname;                           \
    g_structs.at(#sname).vars.at(#vname).size = sizeof(sname.vname);                               \
    g_structs.at(#sname).vars.at(#vname).sizeof_ctype = sizeof(ctype);                             \
    g_structs.at(#sname).vars.at(#vname).offset = (size_t)&sname.vname[0] - (size_t)&sname;        \
    g_structs.at(#sname).vars.at(#vname).name = #vname;                                            \
    g_structs.at(#sname).vars.at(#vname).set = [](const JStringList &args) {                       \
        std::vector<int> indices = get_array_slice_indices(args[1], length);                       \
        for (size_t i = 0; i < indices.size() && i < args.size() - 2; i++) {                       \
            ctype val = cout_##stonum(args[i + 2]);                                                \
            if (g_err.size() == 0) {                                                               \
                sname.vname[indices[i]] = val;                                                     \
            } else {                                                                               \
                return;                                                                            \
            }                                                                                      \
        }                                                                                          \
    };                                                                                             \
    g_structs.at(#sname).vars.at(#vname).print = [](const JStringList &args) {                     \
        auto indices = get_array_slice_indices(args[1], length);                                   \
        for (int i : indices) {                                                                    \
            std::cout << js::fmt(#sname "->" #vname "[%3d] = " printf_fmt "\n", i,                 \
                                 sname.vname[i]);                                                  \
        }                                                                                          \
    };

/*================================================================================*/

//
// make the macros a little smaller
//

static unsigned long cout_stoul_0x(const std::string &str) {
    unsigned long val = js::stoul_0x(str, &g_err);
    if (g_err.length() > 0) {
        std::cout << "Failed " << g_err << "\n";
    }
    return val;
}
static long cout_stol(const std::string &str) {
    long val = js::stol(str, &g_err);
    if (g_err.length() > 0) {
        std::cout << "Failed " << g_err << "\n";
    }
    return val;
}
static double cout_stod(const std::string &str) {
    double val = js::stod(str, &g_err);
    if (g_err.length() > 0) {
        std::cout << "Failed " << g_err << "\n";
    }
    return val;
}

/*================================================================================*/

//
// When setting/printing values from a struct member that is an array,
// this function will return a vector of the indices into the array
// that correspond to the cli input. The input can be single value,
// a comma separated list, or python style slice.
// - [2]
// - [2,3,6]
// - [2:6] --> [2,3,4,5]
//
static std::vector<int> get_array_slice_indices(const std::string &arg, int index_limit) {

    const std::string brace_contents =
        js::contains_all(arg, "[]") ? js::slice(arg, arg.find('[') + 1, arg.find(']')) : ":";

    auto index_stol = [index_limit](const std::string &s) -> int {
        if (s.length() == 0) {
            return 0;
        }

        int i = std::stol(s);
        if (i < 0) {
            i = index_limit + i;
        }

        if (i < 0 || i >= index_limit) {
            throw std::runtime_error{nullptr};
        }

        return i;
    };

    std::vector<int> indices;

    try {

        // python style slicing
        if (js::contains(brace_contents, ":")) {

            const auto brace_split = js::split(brace_contents, ":", js::TrimAll);

            int start = index_stol(brace_split.at(0));
            int stop = index_stol(brace_split.at(1));

            if (stop == 0) {
                stop = index_limit;
            }

            for (int i = start; i < stop; i++) {
                indices.push_back(i);
            }

        } else {
            // csv
            const auto brace_split = js::split(brace_contents, ",", js::TrimAll | js::SkipEmpty);
            for (const std::string &s : brace_split) {

                indices.push_back(index_stol(s));
            }
        }

    } catch (const std::exception & /*e*/) {
        std::cout << "Invalid array indexing: " << brace_contents << "\n";
        return {};
    }

    return indices;
}

/*================================================================================*/

namespace Structs {

struct Var {
    enum class VarType { Std, BField, Array } type;
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
    std::map<std::string, Var> vars;
    size_t working_addr = 0;
};

std::map<std::string, Struct> g_structs;

//
// Try to find a registered struct matchintg the provided name+member name.
//
// A struct name is required, but a member name is not. If no member name is
// provided it will return a pointer to the struct but no var. If a member name
// is provided (without a '*' wildcard) and the member isn't found, returns an error.
//
std::pair<Struct *, Var *> get_struct(const std::string &svname) {

    const auto strs = js::split(svname, "->", js::TrimAll);

    if (strs.size() != 2) {
        std::cout << "Invalid struct name\n";
        return {};
    }

    const std::string sname = strs[0];
    std::string vname = strs[1];

    if (!g_structs.count(sname)) {
        return {};
    }

    Struct *s = &g_structs.at(sname);
    Var *v = nullptr;

    if (js::contains_all(vname, "[]")) {
        js::slice(&vname, 0, vname.find('['));
    }

    if (vname.length() > 0) {

        if (s->vars.count(vname)) {
            // retrieve the sruct member
            v = &g_structs.at(sname).vars.at(vname);
        } else if (vname.back() == '*') {
            // allow wildcard passing through
        } else {
            // if a name was provided but not found, error
            return {};
        }
    }

    return {s, v};
}

/*================================================================================*/

//
// Based on the provided inputs from user, decide what to do with any of
// the structs/struct members we have registered.
//
StructCmdFeedback parse_struct_cmd(const JStringList &args) {

    if (args.size() < 2) {
        std::cout << "Invalid args for struct operation\n";
        return {};
    }

    const auto sv = get_struct(args[1]);
    const auto s = sv.first;
    const auto v = sv.second;

    if (!s) {
        std::cout << "Coulnd't find struct \"" << args[1] << "\"\n";
        return {};
    }

    const bool addr_cmd = (args[0] == "com" || args[0] == "mv");
    const bool read_cmd = (args[0] == "ci" && args.size() == 2);
    const bool write_cmd = (args[0] == "co" && args.size() >= 3);
    const bool src_def_cmd = (args[0] == "struct_src");

    StructCmdFeedback ret{};

    if (addr_cmd && !v && args.size() == 3) {
        size_t new_addr = js::stoul_0x(args[2], &g_err);
        if (g_err.length() > 0) {
            std::cout << "set struct addr: " << g_err << "\n";
        } else {
            s->working_addr = new_addr;
            ret.op = PASS;
        }
    }

    if (read_cmd) {

        ret.op = PRINT;
        if (v) {
            ret.data = v->data;
            ret.size = v->size;
            ret.offset = v->offset + s->working_addr;
            ret.print = v->print;
        } else {
            ret.data = s->data;
            ret.size = s->size;
            ret.offset = s->working_addr;
            ret.print = s->print;
        }
    }

    if (v && write_cmd) {
        ret.data = v->data;
        ret.size = v->size;
        ret.offset = v->offset + s->working_addr;
        ret.set_val = v->set;

        switch (v->type) {
        case Var::VarType::Std:
            ret.op = WRITE;
            break;
        case Var::VarType::Array:
        case Var::VarType::BField:
            ret.op = READ_WRITE;
            break;
        }
    }

    if (src_def_cmd) {
        std::cout << " -> struct " << s->type << " \"" << s->name << "\" definition from "
                  << s->src_filepath << ":\n\n"
                  << s->src_definition << "\n\n";

        ret.op = PASS;
    }

    if (ret.op == ERROR) {
        std::cout << "Invalid args for struct operation\n";
    }

    return ret;
}

/*================================================================================*/

#ifndef COMPILE_EPOCH
#define COMPILE_EPOCH LONG_MAX
#endif

#include "pycstruct_instances.txt"

void init_structs() {

#include "pycstruct_macros.txt"

    JStringList mod_time_warnings;
    struct stat struct_stat;

    // Go through all the structs that have been registered and see if any of
    // their source files have modification times after the struct macros were
    // generated and compiled.
    for (const auto &_s : g_structs) {
        const auto &s = _s.second;

        if (stat(s.src_filepath.c_str(), &struct_stat) < 0) {
            std::cout << "Cannot stat source for " << s.type << " \"" << s.name << "\" ("
                      << s.src_filepath << "): " << strerror(errno) << "\n";
            continue;
        }

        if (struct_stat.st_mtim.tv_sec > COMPILE_EPOCH) {
            mod_time_warnings.push_back("struct " + s.type + " \"" + s.name +
                                        "\": " + s.src_filepath + " last modified " +
                                        DateTime::date_str(struct_stat.st_mtim.tv_sec) + " " +
                                        DateTime::time_str(struct_stat.st_mtim.tv_sec));
        }
    }

    if (mod_time_warnings.size()) {
        std::cout
            << "Source files of structs have been modified since this program was compiled on "
            << DateTime::date_str(COMPILE_EPOCH) + " " + DateTime::time_str(COMPILE_EPOCH) << "\n"
            << js::join(mod_time_warnings, "\n", " -> ") << "\n";
    }
}

} // namespace Structs
