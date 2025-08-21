#include <cstdint>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

#include "../jstrings/jstrings.hpp"
#include "json.hpp"

namespace JsonTools {
using nlohmann::json;

/*
 *
 * Open a file and parse it as json
 *
 */
inline json read(std::string path, std::string *err = nullptr) noexcept {
    if (err)
        *err = "";
    try {
        std::ifstream is(path);

        if (!is.is_open()) {
            std::string _err = "JSON \"" + path + "\": " + std::string{strerror(errno)};

            if (err)
                *err = _err;
            else
                std::cout << _err << "\n";
            return {};
        }

        return json::parse(is, nullptr, true, true, true);

    } catch (const std::exception &e) {
        std::string _err = "JSON READ: " + std::string{e.what()};

        if (err)
            *err = _err;
        else
            std::cout << _err << "\n";
    }
    return {};
}

/*
 * Check that all keys in the json exist.
 *
 * A 'keyset' is a list of keys that signify nested json
 * objects. So to check if a value exists at json["a"]["b"]["c"],
 * the key set would be {"a", "b", "c"}.
 *
 */
using key_list_t = std::vector<JStringList>;
inline std::string veriify_keys(const json &json_obj, const key_list_t &keys) {
    namespace js = JStrings;

    // accumulate all the keys that don't get found
    JStringList missing_keys;

    // walk down through the nested json nodes
    const json *ptr = nullptr;

    for (const auto &key_stack : keys) {
        ptr = &json_obj; // reset to the base node

        // For each key in the keyset, if the key
        // exists then move ptr to the next node
        // in the set until all keys have been
        // checked
        for (const std::string &key : key_stack) {
            if (!ptr->contains(key)) {
                missing_keys.push_back(js::join(key_stack, "", "[", "]"));
                break;
            }
            ptr = &ptr->at(key);
        }
    }

    return js::join(missing_keys, ";");
}

/*
 * Check that all keys in the json exist and that all values
 * referenced by the keysets are the correct type.
 *
 * A 'keyset' is a list of keys that signify nested json
 * objects. So to check if a value exists at json["a"]["b"]["c"],
 * the key set would be {"a", "b", "c"}.
 *
 * The 'type' needs to match the string returned by the
 * json library (json.hpp:24351, 'type_name()').
 *
 * Quick reference
 * - "object"
 * - "array"
 * - "string"
 * - "boolean"
 * - "uint"  |
 * - "int"   | "number" superset
 * - "float" |
 */
using keytype_list_t = std::vector<std::pair<JStringList, std::string>>;

inline std::string verify_key_types(const json &json_obj, const keytype_list_t &keysets) {
    namespace js = JStrings;

    JStringList missing_keys;
    const json *ptr = nullptr;

    auto uint_check = [](json::value_t real_type) -> bool {
        return real_type == json::value_t::number_unsigned;
    };
    auto int_check = [uint_check](json::value_t real_type) -> bool {
        return uint_check(real_type) || real_type == json::value_t::number_integer;
    };
    auto float_check = [int_check](json::value_t real_type) -> bool {
        return int_check(real_type) || real_type == json::value_t::number_float;
    };

    for (auto keytype : keysets) {
        if (keytype.first.size() == 0)
            continue;

        // define the errors ahead of time
        std::string missing_err = "Missing " + js::join(keytype.first, "", "[", "]");
        std::string type_err =
            js::join(keytype.first, "", "[", "]") + " expected " + keytype.second + ", got ";

        // pop off the key of the actual value
        std::string end_key = keytype.first.back();
        keytype.first.pop_back();

        // reset to base node
        ptr = &json_obj;

        // For each key in the keyset, if the key
        // exists then move ptr to the next node
        // in the set until all keys have been
        // checked
        for (const std::string &key_l : keytype.first) {

            if (!ptr->contains(key_l)) {
                missing_keys.push_back(missing_err);
                ptr = nullptr;
                break;
            }
            ptr = &ptr->at(key_l);
        }

        if (ptr) {
            if (!ptr->contains(end_key)) {
                missing_keys.push_back(missing_err);
            } else {
                // if the target key exists, check the provided type string
                // against what the json library has found
                const json::value_t real_type = ptr->at(end_key).type();
                if (keytype.second == "uint" && uint_check(real_type))
                    continue;
                else if (keytype.second == "int" && int_check(real_type))
                    continue;
                else if (keytype.second == "float" && float_check(real_type))
                    continue;
                else if (keytype.second == ptr->at(end_key).type_name())
                    continue;

                missing_keys.push_back(type_err + ptr->at(end_key).type_name());
            }
        }
    }
    return js::join(missing_keys, ";");
}

/*
 * Try and get a value from the json, but if
 * the value doesn't exist don't error and
 * return the provided default
 */
template <typename T> inline T get_default(const json &json, std::string key, T default_val) {

    if (!json.contains(key)) {
        return default_val;
    }
    try {
        return json.at(key).get<T>();
    } catch (const json::exception &e) {
        std::cout << "Json key \"" << key << "\": " << e.what() << "\n";
    }
    return default_val;
}

} // namespace JsonTools
