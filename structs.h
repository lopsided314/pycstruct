#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#define REGISTER_STRUCT(structtype, structname)                                \
  g_structs[#structname] =                                                     \
      Struct{.data = &structname,                                              \
             .size = sizeof(structname),                                       \
             .name = #structname,                                              \
             .type = #structtype,                                              \
             .print = [&structname](void) { printf(#structname " print\n"); }, \
             .vars = {},                                                       \
             .bfields = {}};

#define REGISTER_VAR(structname, varname, ctype, printf_fmt)                   \
  assert(g_structs.count(#structname) &&                                       \
         "Trying to register var with unregistered struct");                   \
  g_structs[#structname].vars[#varname] =                                      \
      Var{.data = &structname.varname,                                         \
          .size = sizeof(structname.varname),                                  \
          .offset = (size_t)&structname.varname - (size_t)&structname,         \
          .name = #varname,                                                    \
          .set =                                                               \
              [](std::string arg) {                                 \
                ctype val = std::stol(arg);                                    \
                structname.varname = val;                                      \
              },                                                               \
          .print =                                                             \
              [](void) {                                            \
                printf(#structname "->" #varname " = " #printf_fmt "\n",       \
                       structname.varname);                                    \
              }};

#define REGISTER_BITFIELD(structname, varname, ctype, printf_fmt)              \
  assert(g_structs.count(#structname) &&                                       \
         "Trying to register var with unregistered struct");                   \
  g_structs[#structname].bfields[#varname] = Bitfield_field{                   \
      .name = #varname,                                                        \
      .set =                                                                   \
          [&structname](std::string arg) {                                     \
            ctype val = std::stol(arg);                                        \
            structname.varname = val;                                          \
          },                                                                   \
      .print =                                                                 \
          [&structname](void) {                                                \
            printf(#structname "->" #varname " = " #printf_fmt "\n",           \
                   structname.varname);                                        \
          }};
          
// namespace Structs
// {
    struct Var {
    void *data;
    size_t size;
    size_t offset;
    std::string name;
    std::function<void(std::string)> set;
    std::function<void(void)> print;
    };

    struct Bitfield_field {
    std::string name;
    std::function<void(std::string)> set;
    std::function<void(void)> print;
    };

    struct Struct {
    void *data;
    size_t size;
    std::string name;
    std::string type;
    std::function<void(void)> print;
    std::map<std::string, Var> vars;
    std::map<std::string, Bitfield_field> bfields;
    };

    extern std::map<std::string, Struct> g_structs;

    void init();
// }
