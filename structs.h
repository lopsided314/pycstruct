

#include <string>

// used by pycstruct
#define REGISTER_STRUCT(a, b) 

void init_structs();
void print_var(std::string structname, std::string varname, std::string extra_args = "");
void set_var(std::string structname, std::string varname, std::string args);

struct StructData {
    void *data;
    size_t size;
};

StructData get_data(std::string structname, std::string varname);


