#ifndef INCLUDE_JSTRINGS_HPP
#define INCLUDE_JSTRINGS_HPP

#include <algorithm> // transform, find_if, remove_if
#include <cassert>
#include <cstdarg> // sprintf
#include <cstdint>
#include <deque>     // jstringlist
#include <stdexcept> // invlaid_argument, out_of_range
#include <string.h>  // memcpy
#include <string>

using JStringList = std::deque<std::string>;

namespace JStrings {

/*===========================================================================*/

inline std::string sprintf(std::string fmt, ...) {
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

    if (!isgraph(pad_char))
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

    if (!isgraph(pad_char))
        pad_char = ' ';

    *str += std::string(new_size - str->length(), pad_char);
}
inline std::string right_pad(std::string str, int new_size, char pad_char = ' ') {
    JStrings::right_pad(&str, new_size, pad_char);
    return str;
}

/*===========================================================================*/

// TODO: Center pad

/*===========================================================================*/

enum StripBehavior : int { Left = 1, Right = 2, Both = Left | Right };

inline void strip(std::string *str, std::string char_set, StripBehavior sb = Both) {
    assert(str != nullptr);
    if (sb & Left) {

        str->erase(str->begin(), std::find_if(str->begin(), str->end(), [char_set](char c) {
                       return char_set.find(c) == std::string::npos;
                   }));
    }
    if (sb & Right) {

        str->erase(
            std::find_if(str->rbegin(), str->rend(),
                         [char_set](char c) { return char_set.find(c) == std::string::npos; })
                .base(),
            str->end());
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

const static int32_t end = INT32_MAX;
inline void slice(std::string *str, int start, int stop) {
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

// replace
enum ReplaceBehavior : int { First = 1, Last = 2, All = First | Last };

inline void replace(std::string *str, std::string key, std::string sub, int rb = All) {
    assert(str != nullptr);

    if (str->find(key) == std::string::npos || key.length() == 0)
        return;

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

enum SplitBehavior : int { None = 0, SkipEmpty = 1, TrimAll = 2 };

inline JStringList split(std::string str, std::string key, int sb = None) {
    if (str.find(key) == std::string::npos) {
        return {str};
    }

    JStringList strs;

    if (key.length() == 0) {
        for (char c : str) {
            strs.push_back({c});
        }
        return strs;
    }

    size_t start = 0;
    size_t stop = 0;
    while ((start = str.find(key, stop)) != std::string::npos) {
        strs.push_back(str.substr(stop, start - stop));
        stop = start + key.length();
    }
    strs.push_back(str.substr(stop, std::string::npos));

    if (sb & TrimAll) {
        for (std::string &s : strs) {
            JStrings::strip(&s);
        }
    }
    if (sb & SkipEmpty) {
        strs.erase(std::remove_if(strs.begin(), strs.end(),
                                  [](const std::string &s) { return s.length() == 0; }),
                   strs.end());
    }

    return strs;
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

inline unsigned long stoul_0x(std::string str, std::string *err = nullptr) {
    if (err)
        *err = "stoul_0x(\"" + str + "\"): ";

    unsigned long val = 0;
    size_t idx = 0;

    try {
        val = std::stoul(str, &idx, 16);
    } catch (const std::invalid_argument &) {
        if (err)
            *err += "invalid_argument";
        return val;
    } catch (const std::out_of_range &) {
        if (err)
            *err += "out_of_range";
        return val;
    }

    if (idx != str.length()) {
        if (err)
            *err += "invalid_argument";
        return val;
    }

    if (err)
        *err = "";
    return val;
}

inline long stol(std::string str, std::string *err = nullptr) {
    if (err)
        *err = "stol(\"" + str + "\"): ";

    long val = 0;
    size_t idx = 0;

    try {
        val = std::stol(str, &idx);
    } catch (const std::invalid_argument &) {
        if (err)
            *err += "invalid_argument";
        return val;
    } catch (const std::out_of_range &) {
        if (err)
            *err += "out_of_range";
        return val;
    }

    if (idx != str.length()) {
        if (err)
            *err += "invalid_argument";
        return val;
    }

    if (err)
        *err = "";
    return val;
}

inline double stod(std::string str, std::string *err = nullptr) {

    if (err)
        *err = "stod(\"" + str + "\"): ";
    double val = 0;
    int exponent = 0;
    size_t idx = 0;

    JStrings::replace(&str, "E", "e");
    try {
        if (JStrings::contains(str, "e")) {
            JStringList spl = JStrings::split(str, "e", TrimAll);
            if (spl.size() != 2) {
                if (err)
                    *err += "Invalid scientific notation";
                return val;
            }

            val = std::stod(spl[0], &idx);
            if (idx != spl[0].length()) {
                if (err)
                    *err += "invalid mantissa";
                return val;
            }

            exponent = std::stol(spl[1], &idx);
            if (idx != spl[1].length()) {
                if (err)
                    *err += "invalid exponent";
                return val;
            }
            double base = exponent > 0 ? 10 : .1;
            exponent = abs(exponent);

            while (exponent-- > 0) {
                val *= base;
            }
        } else {
            val = std::stod(str, &idx);
            if (idx != str.length()) {
                if (err)
                    *err += "invalid_argument";
                return val;
            }
        }
    } catch (const std::invalid_argument &) {
        if (err)
            *err += "invalid_argument";
        return val;
    } catch (const std::out_of_range &) {
        if (err)
            *err += "out_of_range";
        return val;
    }

    if (err)
        *err = "";
    return val;
}

/*===========================================================================*/

template <typename Number_t> inline std::string as_bin(Number_t val) {
    static_assert(sizeof(Number_t) <= 8, "Binary format not supported");
    uint64_t data = 0;
    memcpy(&data, &val, sizeof(val));

    size_t str_end = sizeof(val) * 8 + sizeof(val) - 1 + 1;
    char buf[64] = {0};
    for (int iBit = 0, iBuf = str_end - 2; iBit < sizeof(val) * 8 && iBuf >= 0; iBit++, iBuf--) {
        if (iBit > 0 && (iBit % 8 == 0)) {
            buf[iBuf] = '_';
            iBuf--;
        }

        buf[iBuf] = (data & (1UL << iBit)) > 0 ? '1' : '0';
    }
    return buf;
}

} // namespace JStrings

#endif // INCLUDE_JSTRINGS_HPP
