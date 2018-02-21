#ifndef _ZJ_TYPES_H
#define _ZJ_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	ZJTError,

	ZJTArray=1,
	ZJTBool,
	ZJTNull,
	ZJTNum,
	ZJTObj,
	ZJTStr,
	ZJTRef,

	ZJTEnd,

	ZJNBool,
	ZJNChar,
	ZJNUChar,
	ZJNShort,
	ZJNUShort,
	ZJNInt,
	ZJNUInt,
	ZJNLong,
	ZJNULong,
	ZJNFloat,
	ZJNLongLong,
	ZJNULongLong,
	ZJNDouble,
	ZJNUDouble,
	ZJNLongDouble,
	ZJNULongDouble,
	ZJNVoid,

	ZJNBoolPtr,
    ZJNCharPtr,
    ZJNUCharPtr,
    ZJNShortPtr,
    ZJNUShortPtr,
    ZJNIntPtr,
    ZJNUIntPtr,
    ZJNLongPtr,
    ZJNULongPtr,
    ZJNFloatPtr,
    ZJNLongLongPtr,
    ZJNULongLongPtr,
    ZJNDoublePtr,
    ZJNUDoublePtr,
    ZJNLongDoublePtr,
	ZJNULongDoublePtr,
	ZJNVoidPtr,

	ZJNRef,
	ZJNVal,

	ZJEnd
} ZJType;

typedef struct ZJVal ZJVal;

#define TYPE(obj) _Generic((obj),\
    ZJTA: ZJTArray, \
    ZJTO: ZJTObj, \
    bool: ZJNBool, \
	char: ZJNChar,\
	unsigned char: ZJNUChar,\
	short: ZJNShort,\
	unsigned short: ZJNUShort,\
	int: ZJNInt,\
	unsigned int: ZJNUInt,\
	long: ZJNLong,\
	unsigned long: ZJNULong,\
	long long: ZJNLongLong,\
	unsigned long long: ZJNULongLong,\
	float: ZJNFloat,\
	double: ZJNDouble,\
	long double: ZJNLongDouble,\
	ZJVal*: ZJTRef,\
	ZJVal**: ZJNRef,\
	const ZJVal**: ZJNVal,\
    bool*: ZJNBoolPtr, \
	char*: ZJNCharPtr,\
	unsigned char*: ZJNUCharPtr,\
	short*: ZJNShortPtr,\
	unsigned short*: ZJNUShortPtr,\
	int*: ZJNIntPtr,\
	unsigned int*: ZJNUIntPtr,\
	long*: ZJNLongPtr,\
	unsigned long*: ZJNULongPtr,\
	long long*: ZJNLongLongPtr,\
	unsigned long long*: ZJNULongLongPtr,\
	float*: ZJNFloatPtr,\
	double*: ZJNDoublePtr,\
	long double*: ZJNLongDoublePtr,\
	void*: ZJNVoidPtr\
)

#define TYPE_FMT(x) _Generic((x), \
    char: "%c", \
    signed char: "%hhd", \
    unsigned char: "%hhu", \
    signed short: "%hd", \
    unsigned short: "%hu", \
    signed int: "%d", \
    unsigned int: "%u", \
    long int: "%ld", \
    unsigned long int: "%lu", \
    long long int: "%lld", \
    unsigned long long int: "%llu", \
    float: "%f", \
    double: "%f", \
    long double: "%Lf", \
    char *: "%s", \
    void *: "%p")

#define IS_OBJ(V) (_Generic(V, ZJVal*:1,default:0))

typedef struct ZJArray ZJArray;
typedef struct ZJObject ZJObject;
typedef struct ZJVal ZJVal;

struct ZJVal {
	ZJType	type;
	_Atomic(int) refcount;
	union {
		bool bval;
		ZJArray *array;
		ZJObject *object;
		const char *string;
	};
};

static inline ZJType zj_type(ZJVal *obj){
	return obj->type;
}

typedef struct {int type;} ZJTA;
typedef struct {int type;} ZJTO;

extern ZJTA Array;
extern ZJTO Object;

#endif
