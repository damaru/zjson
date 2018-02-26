#include "json.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    ZJVal *json;
    int i;

    if (isatty(0)) {
        i = 2;
        json = zj_load(argv[1]);
    } else {
        json = zj_fparse(stdin);
        i = 1;
    }
    if (json) {
        if (argv[i])
            zj_print(zj_lookup(json, argv[i]));
        else
            zj_print(json);
        zj_delete(json);
    }
}
