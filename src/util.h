#ifndef _ZJSON_UTIL_H
#define _ZJSON_UTIL_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static inline void *zj_malloc(size_t s)
{
	void *ret = calloc(1, s);

	if (ret == NULL)
		assert(!"Unable to allocate memory");
	return ret;
}

static inline void zj_free(const void *p)
{
	free((void*)p);
}

static inline void *zj_realloc(void *ptr, size_t s)
{
	void *ret = realloc(ptr, s);

	if (ret == NULL)
		assert(!"Unable to allocate memory");
	return ret;
}

static inline char *zj_strdup(const char *ptr, size_t s)
{
	char *ret = strndup(ptr, s);

	if (ret == NULL)
		assert(!"Unable to allocate memory");
	return ret;
}

#endif
