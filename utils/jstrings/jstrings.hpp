#ifndef INCLUDE_JSTRINGS_HPP
#define INCLUDE_JSTRINGS_HPP

#include <algorithm> // transform, find_if, remove_if
#include <cassert>
#include <cstdarg> // fmt
#include <cstdint>
#include <deque>     // jstringlist
#include <stdexcept> // invlaid_argument, out_of_range
#include <string.h>  // memcpy
#include <string>
#include <type_traits>

using JStringList = std::deque<std::string>;

namespace JStrings {

/*===========================================================================*/

// Same thing as sprintf but with std::strings
inline std::string fmt(std::string fmt, ...) {
    char buf[2000] = {0};
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt.c_str(), args);
    va_end(args);
    return buf;
}

/*===========================================================================*/

inline bool contains(const std::string &str, std::string key) {
    return str.find(key) != std::string::npos;
}

inline bool contains(const JStringList &strs, std::string key) {
    return std::find(strs.begin(), strs.end(), key) != strs.end();
}

/*===========================================================================*/

inline bool contains_any(const std::string &str, std::string chars) {
    for (char c : chars) {
        if (str.find(c) != std::string::npos)
            return true;
    }
    return false;
}

inline bool contains_any(const JStringList &strs, const JStringList &keys) {
    for (const std::string &key : keys) {
        if (std::find(strs.begin(), strs.end(), key) != strs.end())
            return true;
    }
    return false;
}

/*===========================================================================*/

inline bool contains_all(const std::string &str, std::string chars) {
    for (char c : chars) {
        if (str.find(c) == std::string::npos)
            return false;
    }
    return true;
}

inline bool contains_all(const JStringList &strs, const JStringList &keys) {
    for (const std::string &key : keys) {
        if (std::find(strs.begin(), strs.end(), key) == strs.end())
            return false;
    }
    return true;
}

/*===========================================================================*/

inline bool contains_only(const std::string &str, std::string chars) {
    for (char c : str) {
        if (chars.find(c) == std::string::npos)
            return false;
    }
    return true;
}

/*===========================================================================*/

using _glibc_str_func = int(*)(int);
inline bool _glibc_ext(const std::string &str, _glibc_str_func func) {
    return std::all_of(str.begin(), str.end(), func);
}
inline bool isalnum(const std::string &str) { return _glibc_ext(str, ::isalnum); }
inline bool isalpha(const std::string &str) { return _glibc_ext(str, ::isalpha); }
inline bool iscntrl(const std::string &str) { return _glibc_ext(str, ::iscntrl); }
inline bool isdigit(const std::string &str) { return _glibc_ext(str, ::isdigit); }
inline bool isgraph(const std::string &str) { return _glibc_ext(str, ::isgraph); }
inline bool islower(const std::string &str) { return _glibc_ext(str, ::islower); }
inline bool isprint(const std::string &str) { return _glibc_ext(str, ::isprint); }
inline bool ispunct(const std::string &str) { return _glibc_ext(str, ::ispunct); }
inline bool isspace(const std::string &str) { return _glibc_ext(str, ::isspace); }
inline bool isupper(const std::string &str) { return _glibc_ext(str, ::isupper); }
inline bool isxdigit(const std::string &str) { return _glibc_ext(str, ::isxdigit); }
inline bool isascii(const std::string &str) { return _glibc_ext(str, ::isascii); }
inline bool isblank(const std::string &str) { return _glibc_ext(str, ::isblank); }

/*===========================================================================*/

inline size_t count(const std::string &str, std::string key) {
    size_t start = 0, stop = 0, count = 0;
    while ((start = str.find(key, stop)) != std::string::npos) {
        count++;
        stop = start + key.length();
    }
    return count;
}

inline size_t count(const JStringList &strs, std::string key) {
    size_t count = 0;
    for (const std::string &str : strs) {
        if (str == key)
            count++;
    }
    return count;
}

/*===========================================================================*/

inline void lower(std::string *str) {
    assert(str != nullptr);
    std::transform(str->begin(), str->end(), str->begin(), ::tolower);
}
inline std::string lower(std::string str) {
    JStrings::lower(&str);
    return str;
}

/*===========================================================================*/

inline void upper(std::string *str) {
    assert(str != nullptr);
    std::transform(str->begin(), str->end(), str->begin(), ::toupper);
}
inline std::string upper(std::string str) {
    JStrings::upper(&str);
    return str;
}

/*===========================================================================*/

inline void left_pad(std::string *str, size_t new_size, char pad_char = ' ') {
    assert(str != nullptr);

    if (str->length() >= new_size) {
        return;
    }

    if (!::isgraph(pad_char))
        pad_char = ' ';

    *str = std::string(new_size - str->length(), pad_char) + *str;
}
inline std::string left_pad(std::string str, size_t new_size, char pad_char = ' ') {
    JStrings::left_pad(&str, new_size, pad_char);
    return str;
}

/*===========================================================================*/

inline void right_pad(std::string *str, size_t new_size, char pad_char = ' ') {
    assert(str != nullptr);

    if (str->length() >= new_size) {
        return;
    }

    if (!::isgraph(pad_char))
        pad_char = ' ';

    *str += std::string(new_size - str->length(), pad_char);
}
inline std::string right_pad(std::string str, int new_size, char pad_char = ' ') {
    JStrings::right_pad(&str, new_size, pad_char);
    return str;
}

/*===========================================================================*/

inline void center_pad(std::string *str, size_t new_size, char pad_char = ' ') {
    assert(str != nullptr);

    if (str->length() >= new_size)
        return;

    if (!::isgraph(pad_char))
        pad_char = ' ';

    size_t pad_length = new_size - str->length();
    size_t right_length = pad_length / 2;
    size_t left_length = pad_length / 2 + pad_length % 2;
    *str = std::string(left_length, pad_char) + *str + std::string(right_length, pad_char);
}
inline std::string center_pad(std::string str, int new_size, char pad_char = ' ') {
    JStrings::center_pad(&str, new_size, pad_char);
    return str;
}

/*===========================================================================*/

//
// Remove all characters in char_set from the left/right/both sides of str. Does
// the same as the python str.strip()
//

enum StripBehavior : int { Left = 1, Right = 2, Both = Left | Right };

inline void strip(std::string *str, const std::string& char_set, StripBehavior sb = Both) {
    assert(str != nullptr);

    auto not_in_chars = [char_set](char c) -> bool {
        return char_set.find(c) == std::string::npos;
    };

    if (sb & Left) {
        str->erase(str->begin(), std::find_if(str->begin(), str->end(), not_in_chars));
    }
    if (sb & Right) {
        str->erase(std::find_if(str->rbegin(), str->rend(), not_in_chars).base(), str->end());
    }
}
inline std::string strip(std::string str, std::string char_set, StripBehavior sb = Both) {
    JStrings::strip(&str, char_set, sb);
    return str;
}

static const char _whitespace[] = " \t\n\r\v\f";
inline void strip(std::string *str, StripBehavior sb = Both) {
    JStrings::strip(str, _whitespace, sb);
}
inline std::string strip(std::string str, StripBehavior sb = Both) {
    JStrings::strip(&str, _whitespace, sb);
    return str;
}

/*===========================================================================*/

//
// String slicing
//
// Supports negative indexing, works the same way as Python strings
//

const static int32_t end = INT32_MAX;
inline void slice(std::string *str, int32_t start, int32_t stop) {
    assert(str != nullptr);
    const int32_t len = str->length();

    if (start < 0) {
        start = len + start;
        if (start < 0)
            start = 0;
    }

    if (stop < 0) {
        stop = len + stop;
        if (stop < 0)
            stop = 0;
    }

    if (start <= 0 && stop >= len)
        return;

    if (stop <= start) {
        *str = "";
    } else {
        *str = str->substr(start, stop - start);
    }
}

inline std::string slice(std::string str, int start, int stop) {
    JStrings::slice(&str, start, stop);
    return str;
}

/*===========================================================================*/

enum ReplaceBehavior : int { First = 1, Last = 2, All = First | Last };

inline void replace(std::string *str, std::string key, std::string sub, int rb = All) {
    assert(str != nullptr);

    if (str->find(key) == std::string::npos || key.length() == 0) {
        return;
    }

    bool no_copy = (key.length() == sub.length());

    if (rb == All) {
        size_t start = 0;
        size_t stop = 0;

        std::string rebuilt;
        while ((start = str->find(key, stop)) != std::string::npos) {
            if (no_copy) {
                for (size_t i = 0; i < key.length(); i++) {
                    str->at(start + i) = sub[i];
                }
            } else {
                rebuilt += str->substr(stop, start - stop) + sub;
            }
            stop = start + key.length();
        }
        if (!no_copy) {
            rebuilt += str->substr(stop, std::string::npos);
            *str = rebuilt;
        }
        return;
    }

    if (rb & First) {
        size_t ifirst = str->find(key);
        if (no_copy) {
            for (size_t i = 0; i < key.length(); i++) {
                str->at(ifirst + i) = sub[i];
            }
        } else {
            *str = str->substr(0, ifirst) + sub +
                   str->substr(ifirst + key.length(), std::string::npos);
        }
    }
    if (rb & Last) {
        size_t ilast = str->rfind(key);
        if (no_copy) {
            for (size_t i = 0; i < key.length(); i++) {
                str->at(ilast + i) = sub[i];
            }
        } else {
            *str =
                str->substr(0, ilast) + sub + str->substr(ilast + key.length(), std::string::npos);
        }
    }
}
inline std::string replace(std::string str, std::string key, std::string sub, int rb = All) {
    JStrings::replace(&str, key, sub, rb);
    return str;
}

/*===========================================================================*/

inline void remove(std::string *str, std::string key, int rb = All) {
    assert(str != nullptr);
    JStrings::replace(str, key, "", rb);
}
inline std::string remove(std::string str, std::string key, int rb = All) {
    JStrings::remove(&str, key, rb);
    return str;
}

/*===========================================================================*/

enum SplitBehavior : int { None = 0, SkipEmpty = 1, TrimAll = 2, SkipWhiteSpace = 4 };

inline JStringList split(std::string str, std::string key, int sb = None, int maxsplit = -1) {
    if (str.find(key) == std::string::npos || maxsplit == 0) {
        if (sb & TrimAll) {
            JStrings::strip(&str);
        }
        if (sb & SkipEmpty && str.length() == 0) {
            return {};
        }
        if (sb & SkipWhiteSpace && JStrings::isspace(str)) {
            return {};
        }
        return {str};
    }

    JStringList strs;
    if (maxsplit < 0) {
        maxsplit = INT32_MAX;
    }

    if (key.length() == 0) {
        int i;
        for (i = 0; i < maxsplit && i < (int)str.length(); i++) {
            strs.push_back({str[i]});
        }
        if (i < (int)str.length() - 1) {
            strs.push_back(str.substr(i, std::string::npos));
        }
        return strs;
    }

    size_t start = 0;
    size_t stop = 0;
    while ((start = str.find(key, stop)) != std::string::npos) {
        strs.push_back(str.substr(stop, start - stop));
        stop = start + key.length();

        if (--maxsplit <= 0) {
            break;
        }
    }
    strs.push_back(str.substr(stop, std::string::npos));

    if (sb & TrimAll) {
        for (std::string &s : strs) {
            JStrings::strip(&s);
        }
    }
    if (sb & SkipEmpty) {
        auto is_empty = [](const std::string &str) -> bool { return str.length() == 0; };
        strs.erase(std::remove_if(strs.begin(), strs.end(), is_empty), strs.end());
    }
    if (sb & SkipWhiteSpace) {
        strs.erase(std::remove_if(strs.begin(), strs.end(), JStrings::isspace), strs.end());
    }

    return strs;
}

/*===========================================================================*/

inline std::string repeat(std::string str, size_t count) {
    std::string ret;
    for (size_t i = 0; i < count; i++) {
        ret += str;
    }
    return ret;
}

inline JStringList repeat(JStringList strs, size_t count) {
    JStringList ret;
    for (size_t i = 0; i < count; i++) {
        for (std::string str : strs) {
            ret.push_back(str);
        }
    }
    return ret;
}

/*===========================================================================*/

inline std::string join(const JStringList &strs, std::string sep = "\n", std::string head = "",
                        std::string tail = "") {
    if (strs.size() == 0)
        return "";

    std::string ret = head + strs.front() + tail;
    for (size_t i = 1; i < strs.size(); i++) {
        ret += sep + head + strs.at(i) + tail;
    }

    return ret;
}

/*===========================================================================*/

inline JStringList combine_lists(const JStringList &list1, const JStringList &list2) {
    JStringList ret;
    for (const std::string &str : list1) {
        ret.push_back(str);
    }
    for (const std::string &str : list2) {
        ret.push_back(str);
    }
    return ret;
}

/*===========================================================================*/

//
// wrapper for std::stoul(..., 16) but with better errors and no exceptions
//
inline unsigned long stoul_0x(std::string str, std::string *err = nullptr) {

    auto set_err = [str, err](const char *exception) -> void {
        if (err)
            *err = "stoul_0x(\"" + str + "\"): " + exception;
    };

    unsigned long val = 0;
    size_t idx = 0;

    try {
        val = std::stoul(str, &idx, 16);
    } catch (const std::invalid_argument &) {
        set_err("invalid_argument");
        return val;
    } catch (const std::out_of_range &) {
        set_err("out_of_range");
        return val;
    }

    if (idx != str.length()) {
        set_err("invalid_argument");
        return val;
    }

    if (err)
        err->clear();
    return val;
}

/*===========================================================================*/

//
// wrapper for std::stol but with better errors and no exceptions
//
inline long stol(std::string str, std::string *err = nullptr) {

    auto set_err = [str, err](const char *exception) -> void {
        if (err)
            *err = "stol(\"" + str + "\"): " + exception;
    };

    long val = 0;
    size_t idx = 0;

    try {
        val = std::stol(str, &idx);
    } catch (const std::invalid_argument &) {
        set_err("invalid_argument");
        return val;
    } catch (const std::out_of_range &) {
        set_err("out_of_range");
        return val;
    }

    if (idx != str.length()) {
        set_err("invalid_argument");
        return val;
    }

    if (err)
        err->clear();

    return val;
}

/*===========================================================================*/

//
// wrapper for std::stod but with better errors and no exceptions. Also
// supports scientific notation, e.g. 2.998e8 or 1.38e-23
//
inline double stod(std::string str, std::string *err = nullptr) {

    auto set_err = [str, err](const char *exception) -> void {
        if (err)
            *err = "stod(\"" + str + "\"): " + exception;
    };

    double val = 0;
    int exponent = 0;
    size_t idx = 0;

    JStrings::replace(&str, "E", "e");
    try {
        if (JStrings::contains(str, "e")) {
            JStringList spl = JStrings::split(str, "e", TrimAll);
            if (spl.size() != 2) {
                set_err("invalid_argument");
                return val;
            }

            val = std::stod(spl[0], &idx);
            if (idx != spl[0].length()) {
                set_err("invalid_argument");
                return val;
            }

            exponent = std::stol(spl[1], &idx);
            if (idx != spl[1].length()) {
                set_err("invalid_argument");
                return val;
            }
            double e = exponent > 0 ? 10 : .1;
            exponent = abs(exponent);

            while (exponent-- > 0) {
                val *= e;
            }
        } else {
            val = std::stod(str, &idx);
            if (idx != str.length()) {
                set_err("invalid_argument");
                return val;
            }
        }
    } catch (const std::invalid_argument &) {
        set_err("invalid_argument");
        return val;
    } catch (const std::out_of_range &) {
        set_err("out_of_range");
        return val;
    }

    if (err)
        err->clear();

    return val;
}

/*===========================================================================*/

//
// Returns the binary representation of a numeric type
//
template <typename Number_t> inline std::string as_bin(Number_t val) {
    static_assert((std::is_arithmetic<Number_t>::value || std::is_pointer<Number_t>::value) &&
                      sizeof(Number_t) <= 8,
                  "Cannot print type as binary");

    // Put the bits into a data type that supports bit operations.
    uint64_t data = 0;
    memcpy(&data, &val, sizeof(val));

    // Move through the bits of the data and set the character in the
    // corresponding index of the string. Inserts visual spacers at
    // set intervals.
    //
    // Indexing reversed for the string so LSB appears on the right.

    char str[100] = {0};
    size_t spacer_spacing = 4;

    // starting index in string
    // (# of bits) + (# of spacers) - (offset due to indexing from 0)
    int iStr = (sizeof(val) * 8) + (sizeof(val) * 8 / spacer_spacing - 1) - 1;

    for (size_t iBit = 0; iBit < sizeof(val) * 8 && iStr >= 0; iBit++) {
        if (iBit > 0 && (iBit % spacer_spacing == 0)) {
            str[iStr--] = '_';
        }

        str[iStr--] = (data & (1UL << iBit)) == 0 ? '0' : '1';
    }
    return str;
}

//
// Try to cram some binary digits into a number
//
template <typename Number_t> inline Number_t from_bin(std::string str, std::string *err) {
    static_assert((std::is_arithmetic<Number_t>::value || std::is_pointer<Number_t>::value) &&
                      sizeof(Number_t) <= 8,
                  "Cannot convert binary to type");

    if (err)
        err->clear();

    JStrings::strip(&str, JStrings::Right);

    // I'll allow 0b1001_1011
    if (!JStrings::contains_only(str, "b0_1")) {
        if (err)
            *err = "Binary numbers can only have 0,1";
        return Number_t{};
    }

    // need to operate on a data type that supports bit operations
    uint64_t data = 0;
    Number_t val{};

    // LSB is at the end of the string
    int iStr = str.length() - 1;

    //
    // Move through the digits in the string, setting the
    // corresponding bit in the number
    //
    for (size_t iBit = 0; iBit < sizeof(val) * 8 && iStr >= 0; iStr--) {

        if (str[iStr] == '_') {
            continue;
        } else if (str[iStr] == 'b') {
            break;
        } else {
            data |= ((str[iStr] == '1') << iBit);
            iBit++;
        }
    }

    // cram the bits into whatever numeric type
    memcpy(&val, &data, sizeof(val));
    return val;
}

} // namespace JStrings

#endif // INCLUDE_JSTRINGS_HPP
