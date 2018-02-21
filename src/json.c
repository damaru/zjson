#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>

#include "structs.h"
#include "util.h"

#define ZJSON_INIT_ALLOC 16

static ZJVal ZJsonVNull = { .type = ZJTNull };
static ZJVal ZJsonVTrue = { .type = ZJTBool, .bval = true };
static ZJVal ZJsonVFalse = { .type = ZJTBool, .bval = false };

ZJTA Array;
ZJTO Object;

ZJVal *const ZJTrue = &ZJsonVTrue;
ZJVal *const ZJFalse = &ZJsonVFalse;
ZJVal *const ZJNull = &ZJsonVNull;

ZJVal *zj_null(void)
{
	return ZJNull;
}

/******************************************************************************/

static inline ZJVal *zj_new_val(ZJType t, const void *p)
{
	ZJVal *ret = zj_malloc(sizeof(ZJVal));

	ret->type = t;
	ret->refcount = 1;
	ret->string = p;
	return ret;
}

void zj_delete(ZJVal *v)
{
	if (!v || --(v->refcount) > 0)
		return;

	if (v->type >= ZJEnd)
		if (ZJCustomTypes[v->type].free)
			ZJCustomTypes[v->type].free(v);

	switch (v->type) {
	case ZJTBool:
	case ZJTNull:
		return;
	case ZJTNum:
	case ZJTStr:
		zj_free((char *)v->string);
		break;
	case ZJTArray:
		for (int i = 0; i < v->array->count; i++)
			zj_delete(v->array->data[i]);
		zj_free(v->array);
		break;
	case ZJTObj:
		for (int i = 0; i < v->object->count; i++) {
			zj_free(v->object->pairs[i].key);
			zj_delete(v->object->pairs[i].value);
		}
		zj_free(v->object);
		break;
	default:
		break;
	}
	zj_free(v);
}

static ZJVal *zj_new_va(ZJType type, va_list va)
{
	ZJVal *v = NULL;

	if (type >= ZJNChar && type <= ZJNLongDouble) {
		const char *fmts[] = {
			[ZJNChar] = "%c",
			[ZJNUChar] = "%hhu",
			[ZJNShort] = "%hd",
			[ZJNUShort] = "%hu",
			[ZJNInt] = "%d",
			[ZJNUInt] = "%u",
			[ZJNLong] = "%ld",
			[ZJNULong] = "%lu",
			[ZJNLongLong] = "%lld",
			[ZJNULongLong] = "%llu",
			[ZJNFloat] = "%f",
			[ZJNDouble] = "%lf",
			[ZJNLongDouble] = "%Lf",
			[ZJNCharPtr] = "%s",
			[ZJNVoidPtr] = "%p",
		};


		const char *fmt = fmts[type];
		char num[256];
		vsnprintf(num, 256, fmt, va);
		num[255] = 0;
		return zj_new_val(ZJTNum, strdup(num));
	}

	switch (type) {
	case ZJTBool:
		v = va_arg(va, bool) ? ZJTrue : ZJFalse;
		break;
	case ZJTNull:
		v = ZJNull;
		break;
	case ZJTNum:
		v = zj_new_val(ZJTNum, strdup(va_arg(va, const char *)));
		break;
	case ZJTStr:
	case ZJNUCharPtr:
	case ZJNCharPtr:
		v = zj_new_val(ZJTStr, strdup(va_arg(va, const char *)));
		break;
	case ZJTArray:
		v = zj_array();
		break;
	case ZJTObj:
		v = zj_object();
		break;
	case ZJNRef:
		v = va_arg(va, ZJVal *);
		break;
	default:
		if (type >= ZJEnd) {
			v = zj_new_val(type, NULL);
			void *data = va_arg(va, void *);
			if (ZJCustomTypes[v->type].init)
				ZJCustomTypes[v->type].init(v, data);
		}
		break;
	}
	return v;
}

ZJVal *zj_new(ZJType type, ...)
{
	va_list va;

	va_start(va, type);
	ZJVal *v = NULL;
	v = zj_new_va(type, va);
	va_end(va);
	return v;
}

ZJVal *zj_ref(const ZJVal *obj)
{
	ZJVal *ret = (ZJVal *)obj;

	ret->refcount++;
	return ret;
}

void zj_unref(ZJVal *obj)
{
	obj->refcount--;
	if (!obj->refcount)
		zj_delete(obj);
}

const char *type_names[] =
{
	[ZJTArray] = "ZJTArray",
	[ZJTBool] = "ZJTBool",
	[ZJTNull] = "ZJTNull",
	[ZJTObj] = "ZJTObj",
	[ZJTStr] = "ZJTStr",
	[ZJTNum] = "ZJTNum",

	[ZJNBool] = "bool",
	[ZJNChar] = "char",
	[ZJNUChar] = "unsigned char",
	[ZJNShort] = "short",
	[ZJNUShort] = "unsigned short",
	[ZJNInt] = "int",
	[ZJNUInt] = "unsigned int",
	[ZJNLong] = "long",
	[ZJNULong] = "unsigned long",
	[ZJNFloat] = "float",
	[ZJNLongLong] = "long long",
	[ZJNULongLong] = "unsigned long long",
	[ZJNDouble] = "double",
	[ZJNUDouble] = "unsigned double",
	[ZJNLongDouble] = "long double",
	[ZJNVoid] = "void",
	[ZJNBoolPtr] = "bool *",
	[ZJNCharPtr] = "char *",
	[ZJNUCharPtr] = "unsigned char *",
	[ZJNShortPtr] = "short *",
	[ZJNUShortPtr] = "unsigned short *",
	[ZJNIntPtr] = "int *",
	[ZJNUIntPtr] = "unsigned int *",
	[ZJNLongPtr] = "long *",
	[ZJNULongPtr] = "unsigned long *",
	[ZJNFloatPtr] = "float *",
	[ZJNLongLongPtr] = "long long *",
	[ZJNULongLongPtr] = "unsigned long long *",
	[ZJNDoublePtr] = "double *",
	[ZJNUDoublePtr] = "unsigned double *",
	[ZJNLongDoublePtr] = "long double *",
	[ZJNVoidPtr] = "void*",
};

bool zj_value(const ZJVal *obj, ZJType atype, ...)
{
	va_list va;

	va_start(va, atype);

	switch (atype) {
	case ZJNBoolPtr:
	{
		bool *baddr = va_arg(va, bool *);
		*baddr = (obj != ZJFalse && obj != ZJNull);
	}
	case ZJNCharPtr:
	case ZJNUCharPtr:
	{
		if (obj->type == ZJTStr || obj->type == ZJTNum) {
			char *caddr = va_arg(va, char *);
			strncpy(caddr, obj->string, strlen(obj->string) + 1);
		} else {
			char *caddr = va_arg(va, char *);
			int len = va_arg(va, int);
			zj_sprint(obj, caddr, len);
		}
	}
	break;
	case ZJNVal:
	{
		ZJVal const **ref = va_arg(va, ZJVal const **);
		*ref = obj;
	}
	break;
	case ZJNRef:
	{
		ZJVal **ref = va_arg(va, ZJVal * *);
		*ref = zj_ref(obj);
	}
	break;
	default:

		if (atype >= ZJNCharPtr && atype <= ZJNVoidPtr && obj->type == ZJTNum) {
			const char *fmts[] = {
				[ZJNShortPtr] = "%hd",
				[ZJNUShortPtr] = "%hu",
				[ZJNIntPtr] = "%d",
				[ZJNUIntPtr] = "%u",
				[ZJNLongPtr] = "%ld",
				[ZJNULongPtr] = "%lu",
				[ZJNLongLongPtr] = "%lld",
				[ZJNULongLongPtr] = "%llu",
				[ZJNFloatPtr] = "%f",
				[ZJNDoublePtr] = "%lf",
				[ZJNLongDoublePtr] = "%Lf",
				[ZJNCharPtr] = "%s",
				[ZJNVoidPtr] = "%p",
			};

			void *addr = va_arg(va, void *);
			sscanf(obj->string, fmts[atype], addr);
		} else {
			fprintf(stderr, "Type Mismatch for %s %s != %s\n", obj->string, type_names[obj->type], type_names[atype]);
			return false;
		}
		break;
	}
	va_end(va);
	return true;
}

/******************************************************************************/

ZJVal *zj_array(void)
{
	int alloc = ZJSON_INIT_ALLOC;
	ZJArray *array = zj_malloc(sizeof(ZJVal) * alloc + sizeof(ZJArray));

	array->count = 0;
	array->alloc = alloc;
	for (int i = 0; i < array->alloc; i++)
		array->data[i] = ZJNull;
	return zj_new_val(ZJTArray, array);
}

int zj_array_count(ZJVal *obj)
{
	return obj->array->count;
}

const ZJVal *zj_array_get(const ZJVal *arr, int index)
{
	assert(arr->type == ZJTArray);
	assert(index < arr->array->count);
	return arr->array->data[index];
}

void zj_array_set_ref(ZJVal *obj, int index, ZJVal *val)
{
	assert(obj->type == ZJTArray);
	ZJArray *arr = obj->array;
	while (index >= arr->alloc) {
		arr = zj_realloc(arr, 2 * arr->alloc * sizeof(ZJVal *) + sizeof(ZJArray));
		for (int i = arr->alloc; i < arr->alloc * 2; i++)
			arr->data[i] = ZJNull;
		arr->alloc *= 2;
		obj->array = arr;
	}

	if (arr->count <= index)
		arr->count = index + 1;

	if (arr->data[index])
		zj_delete(arr->data[index]);
	arr->data[index] = val;
}

void zj_array_set_va(ZJVal *obj, int index, ZJType t, va_list va)
{
	ZJVal *n;

	if (t == ZJTRef)
		n = va_arg(va, ZJVal *);
	else
		n = zj_new_va(t, va);
	zj_array_set_ref(obj, index, n);
}

void zj_array_set(ZJVal *obj, int index, ZJType t, ...)
{
	va_list va;

	assert(obj->type == ZJTArray);
	va_start(va, t);
	zj_array_set_va(obj, index, t, va);
	va_end(va);
}

void zj_array_append_val(ZJVal *obj, ZJVal *val)
{
	zj_array_set_ref(obj, obj->array->count, val);
}

void zj_array_append(ZJVal *obj, ZJType t, ...)
{
	va_list va;

	assert(obj->type == ZJTArray);
	va_start(va, t);
	zj_array_set_va(obj, obj->array->count, t, va);
	va_end(va);
}

void zj_array_foreach(const ZJVal *obj, int (*func)(int, ZJVal *))
{
	for (int i = 0; i < obj->array->count; i++)
		if (func(i, obj->array->data[i]))
			break;
}

/******************************************************************************/

ZJVal *zj_object(void)
{
	int alloc = ZJSON_INIT_ALLOC;
	ZJObject *object = zj_malloc(sizeof(ZJObject) + sizeof(ZJPair) * alloc);

	object->count = 0;
	object->alloc = ZJSON_INIT_ALLOC;
	return zj_new_val(ZJTObj, object);
}

const ZJVal *zj_object_get(const ZJVal *obj, const char *key)
{
	ZJVal *ret = NULL;
	ZJObject *object = obj->object;

	assert(obj->type == ZJTObj);
	for (int i = 0; i < object->count; i++) {
		if (strcmp(key, object->pairs[i].key) == 0) {
			ret = object->pairs[i].value;
			break;
		}
	}
	return ret;
}

void zj_object_clear(ZJVal *obj, const char *key)
{
	assert(obj->type == ZJTObj);
	ZJObject *object = obj->object;
	for (int i = 0; i < object->count; i++) {
		if (strcmp(key, object->pairs[i].key) == 0) {
			ZJVal *old = object->pairs[i].value;
			zj_delete(old);
			zj_free(object->pairs[i].key);
			object->count--;
			for (int j = i; j < object->count; j++)
				object->pairs[j] = object->pairs[j + 1];
			return;
		}
	}
}

void zj_object_set_ref(ZJVal *obj, const char *key, ZJVal *val)
{
	int i;

	assert(obj->type == ZJTObj);
	for (i = 0; i < obj->object->count && obj->object->pairs[i].key; i++) {
		if (strcmp(key, obj->object->pairs[i].key) == 0) {
			if (val != obj->object->pairs[i].value)
				zj_delete(obj->object->pairs[i].value);
			zj_free(obj->object->pairs[i].key);
			obj->object->pairs[i].value = val;
			obj->object->pairs[i].key = key;
			return;
		}
	}

	if (i >= obj->object->alloc) {
		obj->object = zj_realloc(obj->object, sizeof(ZJObject) + obj->object->alloc * 2 * sizeof(ZJPair));
		obj->object->alloc *= 2;
		for (int j = i; j < obj->object->alloc; j++) {
			obj->object->pairs[j].key = NULL;
			obj->object->pairs[j].value = NULL;
		}
	}

	obj->object->pairs[i].key = key;
	obj->object->pairs[i].value = val;
	obj->object->count++;
}

int zj_object_count(ZJVal *obj)
{
	return obj->object->count;
}

void zj_object_set(ZJVal *obj, const char *index, ZJType t, ...)
{
	va_list va;
	ZJVal *n;

	va_start(va, t);
	assert(obj->type == ZJTObj);

	if (t == ZJTRef)
		n = va_arg(va, ZJVal *);
	else
		n = zj_new_va(t, va);

	zj_object_set_ref(obj, strdup(index), n);
	va_end(va);
}

void zj_object_foreach(const ZJVal *obj, int (*func)(const char *, ZJVal *))
{
	for (int i = 0; i < obj->object->count; i++)
		if (func(obj->object->pairs[i].key, obj->object->pairs[i].value))
			break;
}

void zj_object_set_val(ZJVal *obj, const char *key, ZJVal *val)
{
	zj_object_set_ref(obj, strdup(key), zj_ref(val));
}

/******************************************************************************/

/*
	https://tools.ietf.org/html/rfc7396
*/
ZJVal *zj_patch(ZJVal *target, const ZJVal *patch)
{
	if (patch->type == ZJTObj) {
		if (target->type != ZJTObj)
			target = zj_object();

		for (int i = 0; i < patch->object->count; i++) {
			ZJVal *value = patch->object->pairs[i].value;
			const char *key = patch->object->pairs[i].key;
			if (value == ZJNull) {
				zj_object_clear(target, key);
			} else {
				ZJVal *ovalue = (ZJVal *)zj_object_get(target, key);
				zj_object_set_ref(target, strdup(key), zj_patch(ovalue, value));
			}
		}
		return target;
	} else {
		return zj_ref(patch);
	}
}

/******************************************************************************/
static ZJType type_start = ZJEnd;
ZJCustomType ZJCustomTypes[64];

ZJType zjt_register(ZJCustomType tinfo)
{
	ZJType ret = type_start++;

	ZJCustomTypes[ret] = tinfo;
	return ret;
}

void *zjt_get(const ZJVal *v)
{
	return (void *)v->string;
}

ZJVal *zjt_set(ZJVal *v, void *data)
{
	v->string = data;
	return v;
}
/******************************************************************************/
