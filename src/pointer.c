#include "json.h"
#include <ctype.h>
#include <stdlib.h>
#include "structs.h"

static int pointer_decode(char *t, const char *s, size_t len)
{
    bool nondigit = false;

    for (int j = 0; j < len; j++) {
        if (!isdigit(s[j]))
            nondigit = true;
        if (s[j] == '~' && s[j + 1] == '0') {
            *t++ = '~';
            j++;
        } else if (s[j] == '~' && s[j + 1] == '1') {
            *t++ = '/';
            j++;
        } else {
            *t++ = s[j];
        }
    }
    *t = 0;
    return nondigit;
}

static int pointer_tokenize(const char *tok[256], int len[256], const char *p)
{
    int i, count;

    for (i = 0, count = 0; *p; p++, count++) {
        if (*p == '/') {
            len[i - 1] = count;
            tok[i] = p;
            i++;
            count = 0;
        }
        if (i == 255)
            return 0;
    }
    len[i - 1] = count;
    tok[i] = NULL;
    return i;
}

const ZJVal *zj_lookup(const ZJVal *obj, const char *p)
{
    const char *tok[256];
    int len[256];

    if (!pointer_tokenize(tok, len, p))
        return NULL;

    for (int i = 0; tok[i]; i++) {
        char token[len[i] * 2];

        bool nondigit = pointer_decode(token, tok[i] + 1, len[i] - 1);
        if (obj->type == ZJTObj) {
            obj = zj_object_get(obj, token);
        } else if (obj->type == ZJTArray) {
            if (nondigit)
                return ZJNull;
            obj = zj_array_get(obj, atoi(token));
        }
        if (!obj)
            return obj;
    }
    return obj;
}
