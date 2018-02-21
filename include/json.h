#ifndef _LIBZEN_JSON_H
#define _LIBZEN_JSON_H

#include <stdbool.h>
#include <stdio.h>
#include "zjtypes.h"

/*
Memory management notes:

objects of any type allocated with a zj_*_new() should be freed with
zj_delete() function. BUT, if such an object is inserted into *any* container
(ZJsonArray or ZJsonObject) you will be passing the ownership of the object to
the library and you should not free the object.

Note that the library uses a refcount mechanism, so you are free to
reuse objects and insert them into multiple containers and the library keeps
track of its use inside the containers and will be destroyed when the last
container using the object is destroyed.

1. all zj_new starts with refcount == 1
   zj_delete can delete these
2. all freshly allocated objects can be inserted into existing containers using
   zj_*_set_ref api. this api borrows the reference.
3. if you want to inser same object into different container, you need to zj_ref
   over the existing object and insert it.

TODO:
zj_printf()
zj_pack/unpack()
cleanup tests. checkin.
do we need gen.[ch]?
write docx
*/

extern ZJVal * const ZJTrue;
extern ZJVal * const ZJFalse;
extern ZJVal * const ZJNull;

/******************************************************************************/

ZJVal* zj_new(ZJType type, ...);
void zj_delete(ZJVal *v);

ZJVal *zj_ref(const ZJVal* obj);
void zj_unref(ZJVal* obj);

bool zj_value(const ZJVal *obj, ZJType atype, ...);

#define ZJ_NEW(v) zj_new(TYPE(v), v)
#define ZJ_CAST(obj, addr,...) zj_value((obj), TYPE(addr), addr, __VA_ARGS__, 0)

/******************************************************************************/

ZJVal* zj_array(void);
const ZJVal* zj_array_get(const ZJVal *obj, int index);
void zj_array_append(ZJVal *obj, ZJType t, ...);
void zj_array_set(ZJVal *obj, int index, ZJType t, ...);
void zj_array_set_ref(ZJVal *obj, int index, ZJVal *val);
void zj_array_foreach(const ZJVal *obj, int (*func)(int, ZJVal*));

#define ZJA_SET(OBJ, IND, VAL) zj_array_set((OBJ), (IND), TYPE((VAL)), (VAL))
#define ZJA_GET(obj, index, addr) zj_value(zj_array_get(obj, index), TYPE(addr), addr, 0)

#define ZJA_LOOP_BEGIN(obj, k, v)  zj_array_foreach(obj, ({int __fn__(int k, ZJVal * v){
#define ZJA_LOOP_END()  return 0;}; __fn__; }))

/******************************************************************************/

ZJVal* zj_object(void);
const ZJVal* zj_object_get(const ZJVal *obj, const char *key);
void zj_object_set(ZJVal *obj, const char *key, ZJType t, ...);
void zj_object_set_ref(ZJVal *obj, const char *key, ZJVal *val);
void zj_object_clear(ZJVal *obj, const char *key);
void zj_object_foreach(const ZJVal *obj, int (*func)(const char *, ZJVal*));

#define ZJO_SET(OBJ, IND, VAL) zj_object_set((OBJ), (IND), TYPE((VAL)), (VAL))
#define ZJO_GET(obj, index, addr) zj_value(zj_object_get(obj, index), TYPE(addr), addr, 0)

#define ZJO_LOOP_BEGIN(obj, k, v)  zj_object_foreach(obj, ({int __fn__(const char * k, ZJVal * v){
#define ZJO_LOOP_END()  return 0;}; __fn__; }))

/******************************************************************************/

int zj_print(const ZJVal *v);
int zj_dprint(const ZJVal *v, int fd);
int zj_fprint(const ZJVal *v, FILE *fp);
int zj_sprint(const ZJVal *v, char *str, size_t length);
int zj_store(const ZJVal *v, const char *path);

/******************************************************************************/

ZJVal *zj_parse(const char *js);
ZJVal *zj_dparse(int fd);
ZJVal *zj_fparse(FILE *file);
ZJVal *zj_sparsef(const char *jsfmt, ...);
ZJVal *zj_load(const char *path);

/******************************************************************************/

typedef int (*ZJsonWriter)(const void *cookie, const char *data, size_t len);
typedef int (*ZJsonSerialize)(ZJsonWriter printfn, void *cookie, const void *obj);
typedef void (*ZJsonInit)(ZJVal *val, void *data);
typedef void (*ZJsonFree)(ZJVal *val);

typedef struct {
    ZJsonSerialize serialize;
    ZJsonInit init;
    ZJsonFree free;
} ZJCustomType;

void *zjt_get(const ZJVal *v);
ZJVal *zjt_set(ZJVal *v, void *data);
ZJType zjt_register(ZJCustomType tinfo);

/******************************************************************************/

const ZJVal *zj_lookup(const ZJVal *obj, const char *pointer);
ZJVal *zj_patch(ZJVal *target, const ZJVal *patch);

/******************************************************************************/

#define JS(...) (#__VA_ARGS__)
#define J(...) zj_parse(#__VA_ARGS__)
#define JF(fmt, ...) zj_sparsef(fmt,__VA_ARGS__)

#include "gen.h"

#endif /* _LIBZEN_JSON_H */
