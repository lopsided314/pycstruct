#pragma once

#include <cstdio>
#include <string>
#include <time.h>

namespace DateTime {

//
// Return the time as a formatted string
// "hh:mm:ss"
//
// Returns current time if none is provided.
//
inline std::string time_str(time_t time_val = -1) {
    if (time_val < 0) {
        time_val = time(0);
    }

    char buf[100] = {0};
    struct tm *ltime = localtime(&time_val);

    sprintf(buf, "%02d:%02d:%02d", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

    return buf;
}

//
// Return the date as a formatted string
// "yyyy-mm-dd"
//
// Returns current date if none is provided.
//
inline std::string date_str(time_t time_val = -1) {
    if (time_val < 0) {
        time_val = time(0);
    }

    char buf[100] = {0};
    struct tm *ltime = localtime(&time_val);

    sprintf(buf, "%04d-%02d-%02d", ltime->tm_year + 1900, ltime->tm_mon + 1, ltime->tm_mday);

    return buf;
}

} // namespace DateTime
