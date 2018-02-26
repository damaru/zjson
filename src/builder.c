#include <string.h>
#include <json.h>
#include <assert.h>
#include "builder.h"
#include "structs.h"

static inline void process_primitive(ZJBuilder *p, ZJType t, int index, const char *str)
{
    switch (t) {
    case ZJTNull:
        p->value = ZJNull;
        break;
    case ZJTBool:
        p->value = index ? ZJTrue : ZJFalse;
        break;
    case ZJTStr:
        p->value = zj_string(str);
        break;
    case ZJTNum:
        p->value = zj_number(str);
        break;
    default: break;
    }
}

static void tree_builder(ZJBuilder *p, ZBuilderOp op, ZJType t, int index, const char *str)
{
    ZJVal *obj;

    switch (op) {
    case ZJBPrimitive:
        process_primitive(p, t, index, str);
        break;
    case ZJBArrayStart:
        obj = zj_array();
        p->top++;
        assert(p->top < BUILDER_STACK_SIZE);
        p->current = obj;
        p->stack[p->top] = obj;
        p->indent++;
        break;
    case ZJBOArrayValue:
        zj_array_set_ref(p->current, index, p->value);
        break;
    case ZJBArrayEnd:
        p->value = p->current;
        p->top--;
        p->current = p->stack[p->top];
        p->indent--;
        break;
    case ZJBObjectStart:
        obj = zj_object();
        p->top++;
        assert(p->top < BUILDER_STACK_SIZE);
        p->current = obj;
        p->stack[p->top] = obj;
        p->indent++;
        break;
    case ZJBObjectEnd:
        p->value = p->current;
        p->indent--;
        p->top--;
        p->current = p->stack[p->top];
        break;
    case ZJBObjectKey:
        p->kstack[p->top] = strdup(str);
        break;
    case ZJBObjectValue:
        zj_object_set_ref(p->current, p->kstack[p->top], p->value);
        break;
    }
}

ZJBuilder zj_tree_builder(void)
{
    ZJBuilder b = {
        .build  = tree_builder,
        .indent = 0,
        .top    = 0,
    };

    return b;
}
