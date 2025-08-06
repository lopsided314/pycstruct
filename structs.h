

#include <string>
#include "jstrings.hpp"

#define REGISTER_STRUCT(a, b) 

void init_structs();
void print_var(std::string structname, std::string varname, JStringList extra_args = {});
void set_var(std::string structname, std::string varname, JStringList extra_args = {});

struct VarData {
    void *data;
    size_t size;
};

VarData get_data(std::string structname, std::string varname);


