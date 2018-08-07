#include <ctype.h>
#include <json.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>

#include "builder.h"
#include "io.h"
#include "util.h"

typedef struct ZJParser ZJParser;
#define STRING_MAX 1024
#define STRING_DYNAMIC true
#define isutf(c) (((c) & 0xC0) != 0x80)

typedef struct {
    int   count;
    int   length;
    bool  dynamic;
    char *p;
    char  data[STRING_MAX];
} ZJsonStr;

struct ZJParser {
    jmp_buf     context;
    int         line;
    int         col;
    char        prev;
    char        c;
    bool        strcap;
    const char *file;
    ZJsonIn     io;
    ZJsonStr    str;
    char        error_msg[256];
    ZJBuilder   b;
};


static void zj_error(ZJParser *p, const char *fmt, ...);
static ZJType parse(ZJParser *p);
static ZJVal *zen_parser_parse(ZJParser *p);

static inline void str_set(ZJParser *p, char c);
static inline void start_cap(ZJParser *p)
{
    p->str.count = 0;
    if (p->str.dynamic) {
        p->str.length = 16;
        if (!p->str.p)
            p->str.p = malloc(p->str.length);
    }
    str_set(p, p->c);
}

static inline void pause_cap(ZJParser *p)
{
    p->strcap = false;
}

static inline void resume_cap(ZJParser *p)
{
    p->strcap = true;
}

static inline char *end_cap(ZJParser *p)
{
    p->strcap = false;
    if (p->str.count > 0)
        p->str.count--;
    p->str.p[p->str.count] = 0;
    return p->str.p;
}

static inline void str_set(ZJParser *p, char c)
{
    if (p->str.count >= p->str.length - 1) {
        if (p->str.dynamic) {
            p->str.length *= 2;
            p->str.p = realloc(p->str.p, (size_t)p->str.length);
        } else {
            zj_error(p, "Out of memory for string data. Try increasing STING_MAX");
        }
    }
    p->str.p[p->str.count++] = c;
    p->str.p[p->str.count] = 0;
}

static inline void str_back(ZJParser *p)
{
    if (p->str.count > 0)
        p->str.p[--p->str.count] = 0;
}

static inline int advance(ZJParser *p)
{
    p->prev = p->c;
    p->c = p->io.read(&p->io);
    //p->c = _read_char_from_file(&p->io);
    if (p->c == '\n') {
        p->line++;
        p->col = 0;
    }
    p->col++;
    return p->c;
}

static inline int advance_cap(ZJParser *p)
{
    advance(p);
    str_set(p, p->c);
    return p->c;
}

static inline void skip_space(ZJParser *p)
{
    while (p->c && isspace(p->c))
        advance(p);
}

static inline int is_token(ZJParser *p, char c)
{
    if (p->c == c) {
        advance(p);
        return true;
    }
    return false;
}

static inline bool is_literal(ZJParser *p, const char *token)
{
    if (p->c != *token)
        return false;
    const char *t = token;
    for (; *t && p->c == *t; t++)
        advance(p);
    if (*t)
        zj_error(p, "Unexpected character '%c' while parsing token '%s'", p->c, token);
    return true;
}

static inline bool is_in(char c, const char *in)
{
    while (*in) {
        if (*in == c)
            return true;
        in++;
    }
    return false;
}

static inline void zj_error(ZJParser *p, const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    int i = sprintf(p->error_msg, "%s:%d:%d: error: ", p->file, p->line, p->col);
    i += vsnprintf(p->error_msg + i, (255 - i), fmt, va);
    va_end(va);
    longjmp(p->context, -1);
}

static inline void validate_number(ZJParser *p, const char *number, size_t length)
{
    int ocol = p->col;

    p->col -= length;
    if (number[0] == '0' && number[1] != '.' && is_in(number[1], "0123456789e.E"))
        zj_error(p, "malformed number (leading zeros are not allowed)");
    if (number[0] == '-' && !is_in(number[1], "0123456789"))
        zj_error(p, "malformed number (no digits after initial minus)");
    p->col = ocol;
    char last = number[length - 1];
    if (last == '.')
        zj_error(p, "malformed number (no digits after decimal point)");
    if (is_in(last, "eE+-"))
        zj_error(p, "malformed number (no digits after exponent)");
}

static inline ZJType parse_number(ZJParser *p)
{
    start_cap(p);
    //[MINUS]
    if (p->c == '-')
        advance_cap(p);
    //[INT]
    while (p->c >= '0' && p->c <= '9')
        advance_cap(p);
    //[FRAC]
    if (p->c == '.') {
        advance_cap(p);
        while (p->c >= '0' && p->c <= '9')
            advance_cap(p);
    }
    //[EXP]
    if (is_in(p->c, "eE")) {
        advance_cap(p);
        if (is_in(p->c, "+-"))
            advance_cap(p);
        while (p->c >= '0' && p->c <= '9')
            advance_cap(p);
    }
    const char *num = end_cap(p);
    validate_number(p, num, p->str.count);
    skip_space(p);
    return ZJTNum;
}

static inline int to_utf8(char *dest, uint32_t ch)
{
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch >> 6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch >> 12) | 0xE0;
        dest[1] = ((ch >> 6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch >> 18) | 0xF0;
        dest[1] = ((ch >> 12) & 0x3F) | 0x80;
        dest[2] = ((ch >> 6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
}

static inline void handle_escape(ZJParser *p)
{
    char d[5] = { 0, 0, 0, 0, 0 };
    int l = 1;

    str_back(p);
    pause_cap(p);
    advance(p);
    switch (p->c) {
    case '"': *d = '\"'; break;
    case '\\': *d = '\\'; break;
    case '\'': *d = '\''; break;
    case 'b': *d = '\b'; break;
    case 'f': *d = '\f'; break;
    case 'n': *d = '\n'; break;
    case 'r': *d = '\r'; break;
    case 't': *d = '\t'; break;
    case '/': *d = '/'; break;
    case 'u': {
        char num[5] = {
            advance(p),
            advance(p),
            advance(p),
            advance(p),
            0
        };
        for (int j = 0; j < 4; j++)
            if (!isxdigit(num[j]))
                zj_error(p, "Invalid unicode escape '\\u%s'\n", num);
        uint32_t uchar = strtoul(num, NULL, 16);
        l = to_utf8(d, uchar);
    }
    break;
    default:
        zj_error(p, "Invalid escape sequence '\\%c'", p->c);
        break;
    }
    for (int j = 0; j < l; j++)
        str_set(p, d[j]);
    resume_cap(p);
}

static inline ZJType parse_string(ZJParser *p)
{
    start_cap(p);
    while (p->c != '\"') {
        if (p->c == '\\')
            handle_escape(p);
        else if (p->c >= 0x00 && p->c <= 0x1F && p->prev != '\\')
            zj_error(p, "Unescaped control character 0x%x found", p->c);
        advance_cap(p);
    }
    end_cap(p);
    is_token(p, '\"');
    return ZJTStr;
}

static inline void handle_comments(ZJParser *p);
static inline int expect(ZJParser *p, const char *t)
{
    int c = 0;

    skip_space(p);
    if (p->c == '/' && is_token(p, '/')) {
        handle_comments(p);
        skip_space(p);
    }
    if (is_in(p->c, t)) {
        c = p->c;
        advance(p);
    } else if (p->c == 0) {
        zj_error(p, "Unexpected EOF");
    } else {
        zj_error(p, "Unexpected character '%c'. Expecting %s'%s' ", p->c, t[1] ? "one of " : "", t);
    }
    return c;
}

static ZJType parse_object(ZJParser *p)
{
    p->b.build(&p->b, ZJBObjectStart, ZJTObj, 0, p->str.p);
    ZJType ret = ZJTObj;
    skip_space(p);
    while (!is_token(p, '}')) {
        ZJType key = parse(p);
        if (key != ZJTStr)
            zj_error(p, "Only string values are allowed as keys in objects");
        p->b.build(&p->b, ZJBObjectKey, key, 0, p->str.p);
        expect(p, ":");
        ZJType value = parse(p);
        if (key && value) {
            if (value == ZJTStr)
                p->b.build(&p->b, ZJBPrimitive, value, 0, p->str.p);
            p->b.build(&p->b, ZJBObjectValue, value, 0, p->str.p);
            if (expect(p, ",}") == '}')
                break;
        } else {
            zj_error(p, "expecting a value after ':'");
            break;
        }
    }
    p->b.build(&p->b, ZJBObjectEnd, ZJTObj, 0, p->str.p);
    return ret;
}

static ZJType parse_array(ZJParser *p)
{
    int i = 0;

    p->b.build(&p->b, ZJBArrayStart, ZJTArray, 0, p->str.p);
    skip_space(p);
    while (!is_token(p, ']')) {
        ZJType value = parse(p);
        if (value) {
            if (value == ZJTStr)
                p->b.build(&p->b, ZJBPrimitive, value, 0, p->str.p);

            p->b.build(&p->b, ZJBOArrayValue, value, i, p->str.p);
            if (expect(p, ",]") == ',')
                i++;
            else
                break;
        } else {
            zj_error(p, "Unexpected character '%c'. Expecting a value", p->c);
        }
    }
    p->b.build(&p->b, ZJBArrayEnd, ZJTArray, i, p->str.p);
    return ZJTArray;
}

static inline void handle_comments(ZJParser *p)
{
    if (is_token(p, '/')) {
        while (p->c != '\n')
            advance(p);
        advance(p);
    }
    if (is_token(p, '*')) {
        while (!(p->prev == '*' && p->c == '/'))
            advance(p);
        advance(p);
    }
}

static ZJType parse(ZJParser *p)
{
    ZJType ret = -1;

again:
    skip_space(p);
    if (is_token(p, '[')) {
        ret = parse_array(p);
    } else if (p->c == '\"') {
        advance(p);
        ret = parse_string(p);
        //p->b.build(&p->b, ZJBPrimitive, ret, 0, p->str.p);
    } else if (is_token(p, '{')) {
        ret = parse_object(p);
    } else if (is_literal(p, "true")) {
        ret = ZJTBool;
        p->b.build(&p->b, ZJBPrimitive, ret, true, p->str.p);
    } else if (is_literal(p, "false")) {
        ret = ZJTBool;
        p->b.build(&p->b, ZJBPrimitive, ret, false, p->str.p);
    } else if (is_literal(p, "null")) {
        ret = ZJTNull;
        p->b.build(&p->b, ZJBPrimitive, ret, 0, p->str.p);
    } else if (p->c == '-' || (p->c >= '0' && p->c <= '9')) {
        ret = parse_number(p);
        p->b.build(&p->b, ZJBPrimitive, ret, 0, p->str.p);
    } else if (is_token(p, '/')) {
        handle_comments(p);
        goto again;
    } else if (p->c == 0) {
        zj_error(p, "Unexpected EOF\n");
    } else {
        zj_error(p, "Unexpected Character: (%c)\n", p->c);
    }
    return ret;
}

void zj_init_parser(ZJParser *p, const char *path, int line, ZJsonIn io)
{
    p->file = path;
    p->line = line;
    p->col = 1;
    p->strcap = false;
    p->io = io;
    p->str.dynamic = STRING_DYNAMIC;
    if (p->str.dynamic) {
        p->str.p = NULL;
        p->str.length = 0;
    } else {
        p->str.p = p->str.data;
        p->str.length = STRING_MAX;
    }
    p->b = zj_tree_builder();
}

static ZJVal *zen_parser_parse(ZJParser *p)
{
    p->c = p->io.read(&p->io);
    if (setjmp(p->context)) {
        fprintf((stderr), "%s\n", p->error_msg);
        return NULL;
    }
    if (!isutf(p->c))
        zj_error(p, "This parser only expects utf-8 encoded strings");
    if (parse(p) != -1) {
        if (p->str.dynamic)
            free(p->str.p);
        return p->b.stack[p->b.top + 1];
    }
    return NULL;
}

ZJVal *zj_sparsef(const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
#ifdef _GNU_SOURCE
    char *tmp = NULL;
    if (vasprintf(&tmp, fmt, va) < 0)
        return NULL;

#else
    int len = 128;
    char *tmp = zj_malloc(len);
    while (vsnprintf(tmp, len, fmt, va) >= len) {
        len = len * 2;
        tmp = zj_realloc(tmp, len);
        /* vsnprintf mangles va we need to reset it */
        va_end(va);
        va_start(va, fmt);
    }
#endif
    ZJVal *ret = zj_parse(tmp);
    zj_free(tmp);
    va_end(va);
    return ret;
}

ZJVal *zj_dparse(int fd)
{
    char path[64];

    sprintf(path, "fd:%d", fd);
    ZJParser p;
    zj_init_parser(&p, path, 1, zj_fd_reader(fd));
    p.c = p.io.read(&p.io);
    return zen_parser_parse(&p);
}

ZJVal *zj_fparse(FILE *file)
{
    ZJParser p;
    char path[64];

    sprintf(path, "fp:%p", (void *)file);
    ZJsonIn io = zj_file_reader(file);
    zj_init_parser(&p, path, 1, io);
    p.c = p.io.read(&p.io);
    return zen_parser_parse(&p);
}

ZJVal *zj_parse(const char *js)
{
    ZJParser p;

    zj_init_parser(&p, NULL, 1, zj_string_reader(js, strlen(js)));
    return zen_parser_parse(&p);
}

#ifdef NO_MMAP

ZJVal *zj_load(const char *path)
{
    FILE *fp = fopen(path, "r");
    ZJVal *ret = zj_fparse(fp);

    fclose(fp);
    return ret;
}

#else  /* NO_MMAP */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
ZJVal *zj_load(const char *path)
{
    char *buf;
    struct stat sbuf;
    size_t jslen;
    ZJParser p;

    int fd = open(path, O_RDONLY);

    if (stat(path, &sbuf) != 0)
        return NULL;

    jslen = sbuf.st_size + 1;
    buf = mmap(NULL, jslen, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    zj_init_parser(&p, NULL, 1, zj_string_reader(buf, jslen));
    ZJVal *json = zen_parser_parse(&p);

    munmap(buf, jslen);
    return json;
}
#endif /* NO_MMAP */
