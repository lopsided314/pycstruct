#include "json_tools.hpp"

int main() {
    auto json = JsonTools::read("./ab.json");

    JsonTools::keytype_list_t keytypes = {{{"One"}, "object"},
                                          {{"One", "One Array"}, "array"},
                                          {{"One", "One Object", "Two Int"}, "uint"}};

    std::string m = JsonTools::verify_key_types(json, keytypes);
    std::cout << m << "\n";
}
