

#include <string>

#define REGISTER_STRUCT(a, b) 

void init_structs();
void print_var(std::string structname, std::string varname, std::string extra_args = "");
void set_var(std::string structname, std::string varname, std::string args);

struct VarData {
    void *data;
    size_t size;
};

VarData get_data(std::string structname, std::string varname);


