#include "structs.h"

std::map<std::string, Struct> g_structs;

//@struct_instances
static struct _Test { int a ; uint32_t b:10 , :6 , c:12 , :4 ; long long d ; float f ; } _test;

//@struct_instances








void init()
{
    REGISTER_VAR(_test, a, int, i32)
    REGISTER_VAR(_test, d, long long, i64)
    REGISTER_VAR(_test, f, float, f32)
    REGISTER_BITFIELD(_test, b, uint32_t, u32)
    REGISTER_BITFIELD(_test, c, uint32_t, u32)
}

























