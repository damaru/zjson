#ifndef _ZJSON_STRUCTS_H
#define _ZJSON_STRUCTS_H

#include <zjtypes.h>

struct ZJArray {
	int	count;
	int	alloc;
	ZJVal *data[1];
};
typedef struct ZJPair ZJPair;
struct ZJPair {
	ZJPair *next;
	ZJPair *onext;
	ZJPair *oprev;
	ZJVal *value;
	const char *key;
} ;

struct ZJObject {
	int size  ;/**< size of bucket array                */
	int active;/**< Active buckets                      */
	int victim;/**< next bucket to be split             */
	int count ;/**< number of hash elements             */
	ZJPair **buckets;
	ZJPair *first;
	ZJPair *last;
};
extern ZJCustomType ZJCustomTypes[64];

#endif
