#include <json.h>
#include "structs.h"
#include "util.h"
#define ZJSON_INIT_ALLOC 16

static void rehash(ZJObject *hash);

static inline ZJVal *zj_new_val(ZJType t, const void *p)
{
    ZJVal *ret = zj_malloc(sizeof(ZJVal));

    ret->type = t;
    ret->refcount = 1;
    ret->string = p;
    return ret;
}

static unsigned int zj_hash(ZJObject *object, const char *key)
{
    const char *k;
    unsigned int h;

    k = (char *)key;
    for (h = 0; *k; ++k) {
        h *= 16777619;
        h ^= *k;
    }
    h = h % object->size;
    if (h >= object->active)
        h = h - object->size / 2;
    return h;
}

#define LOAD 4

ZJVal *zj_object(void)
{
    int alloc = ZJSON_INIT_ALLOC;
    ZJObject *object = zj_malloc(sizeof(ZJObject));

    object->size = alloc;
    object->active = alloc;
    object->count = 0;
    object->victim = 0;
    object->buckets = zj_malloc(sizeof(ZJPair *) * alloc);
    object->first = NULL;

    return zj_new_val(ZJTObj, object);
}

const ZJVal *zj_object_get(const ZJVal *obj, const char *key)
{
    ZJObject *object = obj->object;

    assert(obj->type == ZJTObj);
    unsigned addr = zj_hash(object, key);

    for (ZJPair *p = object->buckets[addr]; p; p = p->next)
        if (strcmp(key, p->key) == 0)
            return p->value;
    return NULL;
}


void zj_object_clear(ZJVal *obj, const char *key)
{
    ZJObject *object = obj->object;

    assert(obj->type == ZJTObj);
    unsigned addr = zj_hash(object, key);

    for (ZJPair *p = object->buckets[addr], *q = NULL, *n = NULL; p; q = p, p = n) {
        if (strcmp(key, p->key) == 0) {
            if (q)
                q->next = p->next;
            else
                object->buckets[addr] = p->next;
            if (p->oprev)
                p->oprev->onext = p->onext;
            else
                object->first = p->onext;

            if (p->onext)
                p->onext->oprev = p->oprev;
            else
                object->last = p->oprev;
            zj_free(p->key);
            zj_free(p->value);
            n = p->next;
            zj_free(p);
        }
    }
}

void zj_object_set_ref(ZJVal *obj, const char *key, ZJVal *val)
{
    ZJObject *object = obj->object;
    ZJPair *p;

    assert(obj->type == ZJTObj);
    unsigned addr = zj_hash(object, key);
    for (p = object->buckets[addr]; p; p = p->next) {
        if (strcmp(key, p->key) == 0) {
            if (key != p->key)
                zj_free(p->key);
            if (val != p->value)
                zj_delete(p->value);
            p->key = key;
            p->value = val;
            return;
        }
    }

    p = zj_malloc(sizeof(ZJPair));
    p->value = val;
    p->key = key;
    p->next = object->buckets[addr];

    if (object->last) {
        p->oprev = object->last;
        object->last->onext = p;
        object->last = p;
    } else {
        object->first = object->last = p;
        p->oprev = p->onext = NULL;
    }

    object->buckets[addr] = p;
    object->count++;

    if (object->count / object->active > 16)
        rehash(object);

}

int zj_object_count(ZJVal *obj)
{
    return obj->object->count;
}

void zj_object_set(ZJVal *obj, const char *index, ZJVal *n)
{
    zj_object_set_ref(obj, strdup(index), n);
}

void zj_object_foreach(const ZJVal *obj, int (*func)(const char *, ZJVal *, void *), void *cookie)
{
    for (ZJPair *p = obj->object->first; p; p = p->onext)
        if (func(p->key, p->value, cookie))
            return;
}

void zj_object_delete(ZJVal *obj)
{
    ZJPair *q;

    for (int i = 0; i < obj->object->active; i++) {
        for (ZJPair *p = obj->object->buckets[i]; p; p = q) {
            q = p->next;
            zj_free(p->key);
            zj_delete(p->value);
            zj_free(p);
        }
    }
    zj_free(obj->object->buckets);
    zj_free(obj->object);
    //zj_free(obj);
}

//#define REHASH_ALL 0
static void rehash(ZJObject *hash)
{
    unsigned int addr = 0;
    ZJPair *elem = NULL;
    ZJPair *temp;
    ZJPair *next;

    if (hash->active == hash->size) {
        hash->buckets = zj_realloc(hash->buckets, sizeof(ZJPair *) * (hash->size * 2));

        for (addr = hash->size; addr < hash->size * 2; addr++)
            hash->buckets[addr] = NULL;

        hash->size *= 2;
        hash->victim = 0;
        //printf("Table Doubled to %d buckets\n", hash->size);
    }

#ifdef REHASH_ALL
    while (hash->active < hash->size)
#endif
    {
        temp = hash->buckets[hash->victim];

        //printf("victim: %d, acitve %d\n", hash->victim, hash->active);

        hash->buckets[hash->active] = NULL;
        hash->buckets[hash->victim] = NULL;

        hash->active++;

        for (elem = temp; elem; elem = next) {
            next = elem->next;
            addr = zj_hash(hash, elem->key);
            elem->next = hash->buckets[addr];
            hash->buckets[addr] = elem;
            //printf( "rehashing %d from %d to %d\n", *(unsigned *)elem, hash->victim, addr);
        }

        hash->victim++;
    }

    //printf("hash->size %d\n", hash->size);
}
