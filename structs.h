

#include <string>


#define REGISTER_STRUCT(a, b) 

void init_structs();
void print_var(std::string structname, std::string varname);

struct StructData {
    void *data;
    size_t size;
};

StructData get_data(std::string structname, std::string varname);


