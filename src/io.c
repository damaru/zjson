#include <unistd.h>
#include "io.h"
#include "util.h"

static int from_file(ZJsonIn *in)
{
    if (in->count >= in->length - 1) {
        in->length = fread((char *)in->fin, 1, sizeof(in->fin), in->fp);
        in->count = 0;
        in->c = in->next;
        in->next = in->fin[0];
        return in->c;
    }
    in->c = in->next;
    in->count++;
    in->next = in->fin[in->count];
    return in->c;
}

ZJsonIn zj_file_reader(FILE *file)
{
    ZJsonIn io = {
        .fp     = file,
        .read   = from_file,
        .count  = 32,
        .length = 32,
    };

    return io;
}

static int from_fd(ZJsonIn *in)
{
    in->c = in->next;
    if (read(in->fd, &in->next, 1) < 1)
        in->next = 0;
    return in->c;
}

ZJsonIn zj_fd_reader(int fd)
{
    ZJsonIn io = {
        .fd   = fd,
        .read = from_fd
    };

    return io;
}

static int from_string(ZJsonIn *in)
{
    if (in->count >= in->length-1)
        return -EOF;
    in->c = in->next;
    in->count++;
    in->next = in->in[in->count];
    return in->c;
}

ZJsonIn zj_string_reader(const char *js, size_t length)
{
    ZJsonIn io = {
        .in     = js,
        .read   = from_string,
        .count  = 0,
        .length = length,
        .next   = length>1?*js:0
    };

    return io;
}

static int to_string(ZJsonOut *p, const char *source, size_t len)
{
    if (len < p->length - p->count) {
        strncpy(p->out + p->count, source, len);
        p->count += len;
    }
    return len;
}

ZJsonOut zj_string_writer(char *str, size_t length)
{
    ZJsonOut o = {
        .out    = str,
        .length = length,
        .count  = 0,
        .write  = to_string
    };

    return o;
}

int to_file(ZJsonOut *p, const char *str, size_t len)
{
    return fwrite(str, 1, len, p->fp);
}

ZJsonOut zj_file_writer(FILE *fp)
{
    ZJsonOut o = {
        .fp    = fp,
        .write = to_file,
    };

    return o;
}

static int to_fd(ZJsonOut *p, const char *s, size_t len)
{
    return write(p->fd, s, len);
}

ZJsonOut zj_fd_writer(int fd)
{
    ZJsonOut o = {
        .fd    = fd,
        .write = to_fd
    };

    return o;
}
