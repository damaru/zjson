
#ifndef _ZJSON_BUILDER_H
#define _ZJSON_BUILDER_H

typedef enum{
    ZJBArrayStart,
    ZJBArrayEnd,
    ZJBObjectStart,
    ZJBObjectEnd,
    ZJBObjectKey,
    ZJBObjectValue,
    ZJBOArrayValue,
    ZJBPrimitive,
} ZBuilderOp;

#define BUILDER_STACK_SIZE 32

typedef struct ZJBuilder ZJBuilder;
struct ZJBuilder {
    int indent;
    ZJVal *current;
    const char *key;
    ZJVal *value;
    ZJVal *stack[BUILDER_STACK_SIZE];
    char *kstack[BUILDER_STACK_SIZE];
    int top;
    void (*build)(ZJBuilder *p, ZBuilderOp op, ZJType t, int index, const char *str);
};

ZJBuilder zj_tree_builder(void );

#endif
