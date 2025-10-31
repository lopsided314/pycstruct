#pragma once


#include <cstdint>
struct Name2
{ // fasd
    int a;
    int b : 20, :12;
    unsigned int c:12, :20; // a
    float d[4];
    unsigned short e;
    /* this scomment
    alskdfj
    */
    struct PLPL {
        uint32_t att:9, :7, phase:9, :7;
        uint32_t val;
    } plpl;
    union U {
        int i;
        float f;
        uint32_t u;
    } u1;
    char str[100];
};

