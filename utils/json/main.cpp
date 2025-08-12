#include "json_tools.hpp"

int main() {
  auto json = JsonTools::read("./ab.json");

  JsonTools::key_list_t keys = {{"One", "One Array"},
                                   {"One", "One Object", "Two Float"}};

  std::string m = JsonTools::veriify_keys(json, keys);

  JsonTools::keytype_list_t keytypes = {
      {{"One"}, "object"},
      {{"One", "One Array"}, "array"},
      {{"One", "One Object", "Two Float"}, "string"}};

  m = JsonTools::verify_key_types(json, keytypes);
  std::cout << m << "\n";

    std::cout << JsonTools::get_default<bool>(json["One"], "One Int", 0) << "\n";

}
