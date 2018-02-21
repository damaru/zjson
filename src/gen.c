#include <json.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

#define DEFINE_FUNCTIONS(name, type, conv, tenum)                           \
type zj_to_##name(const ZJVal *v){                                          \
    return (conv);                                                          \
}                                                                           \
ZJVal* zj_##name(type b){                                                   \
    return zj_new(tenum, b);                                                \
}                                                                           \
void zj_array_set_##name(ZJVal *obj, int index, type b){                    \
    zj_array_set_ref(obj, index, zj_##name(b));                             \
}                                                                           \
void zj_object_set_##name(ZJVal *obj, const char *key, type b){             \
    zj_object_set_ref(obj, strdup(key), zj_##name(b));                      \
}                                                                           \
type zj_object_get_##name(const ZJVal *obj, const char *key){               \
    return zj_to_##name(zj_object_get(obj, key));                           \
}                                                                           \
type zj_array_get_##name(const ZJVal *obj, int key){                        \
    return zj_to_##name(zj_array_get(obj, key));                            \
}

DEFINE_FUNCTIONS(bool, bool, v->bval, ZJTBool)
DEFINE_FUNCTIONS(double, double, (strtod(v->string, NULL)), ZJNDouble)
DEFINE_FUNCTIONS(float, float, (strtof(v->string, NULL)), ZJNFloat)
DEFINE_FUNCTIONS(int, int, (strtol(v->string, NULL, 10)), ZJNInt)
DEFINE_FUNCTIONS(l,long int, (strtol(v->string, NULL, 10)), ZJNInt)
DEFINE_FUNCTIONS(ll,long long int, (strtoll(v->string, NULL, 10)), ZJNInt)
DEFINE_FUNCTIONS(ld,long double, (strtold(v->string, NULL)), ZJNInt)
DEFINE_FUNCTIONS(string, const char *, (v->string), ZJTStr)
DEFINE_FUNCTIONS(number, const char*,  (v->string), ZJTNum)
