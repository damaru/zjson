#ifndef _ZJSON_STRUCTS_H
#define _ZJSON_STRUCTS_H

#include <zjtypes.h>

struct ZJArray {
	int	count;
	int	alloc;
	ZJVal *data[1];
};

typedef struct {
	ZJVal *value;
	const char *key;
} ZJPair;

struct ZJObject {
	int	count;
	int	alloc;
	ZJPair	pairs[1];
};
extern ZJCustomType ZJCustomTypes[64];

#endif
