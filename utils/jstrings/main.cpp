#include "jstrings.hpp"
#include <iostream>

namespace js = JStrings;

#define TEST(a, b)                                                                                 \
    do {                                                                                           \
        if (a != b) {                                                                              \
            std::cout << "Failed " #a " got: " << a << "\n";                                       \
        }                                                                                          \
    } while (0);

int main() {
    std::string ex = "Example Text";

    TEST(js::lower(ex), "example text");
    TEST(js::upper(ex), "EXAMPLE TEXT");

    TEST(js::left_pad(ex, 4), "Example Text");
    TEST(js::left_pad(ex, 14, '-'), "--Example Text");
    TEST(js::right_pad(ex, 4), "Example Text");
    TEST(js::right_pad(ex, 14, '-'), "Example Text--");
    TEST(js::center_pad(ex, 17, '-'), "---Example Text--");

    TEST(js::slice(ex, 1, 4), "xam");
    TEST(js::slice(ex, 2, -2), "ample Te");
    TEST(js::slice(ex, -4, js::end), "Text");
    TEST(js::slice(ex, -20, 15), "Example Text");

    std::string strip_test = "\n\t    4...3   \n\t";
    TEST(js::strip(strip_test), "4...3");
    TEST(js::strip(strip_test, js::Left), "4...3   \n\t");
    TEST(js::strip(strip_test, js::Right), "\n\t    4...3");

    strip_test = "1234 ..asdfkj  f24513";
    TEST(js::strip(strip_test, "123"), "4 ..asdfkj  f245");
    TEST(js::strip(strip_test, "123", js::Left), "4 ..asdfkj  f24513");

    TEST(js::replace(ex, "e", "!"), "Exampl! T!xt");
    TEST(js::replace(ex, "ampl", "xx"), "Exxxe Text");
    TEST(js::replace(ex, "e", "!", js::Last), "Example T!xt");
    TEST(js::replace(ex + "Text", "Text", "!", js::First), "Example !Text");

    TEST(js::remove(ex, "le "), "ExampText");

    TEST(js::contains_only("0b0110011010", "b01"), true);
    TEST(js::isspace("\t  \n"), true);

    std::string split_test = "ex ex  ex ";
    auto split_ret = js::split(split_test, "x", JStrings::TrimAll, 0);
     // split_ret = js::split(split_test, "", JStrings::TrimAll, 3);
    std::cout << js::join(split_ret, "\n", "\"", "\"") << "\n";
    

    std::string err;
    // short x = js::from_bin<short>("0b000000000000000011", &err);
    // std::cout << x << "\n";

    js::stoul_0x("0x123k", &err);
    if (err.length() > 0) {
        std::cout << err << "\n";
    }

}
