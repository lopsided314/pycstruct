#pragma once


#include <cstdint>

struct Name2
{ // fasd
    int a;
    unsigned long b : 12, :20, c:12, :20; // a
    float d[4];
    unsigned short e;
    /* this scomment
    alskdfj
    */
    struct Sub {
        const char *s;
        struct Sub2 {
            int i2;
        } sub2;
    } sub1;
    union U {
        int i;
        float f;
        uint32_t u:12, :20;
    } u1;
    struct Sub subs[12];
    union U u2;
};

