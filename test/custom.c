#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int x;
    int y;
} point;

int print_point(ZJsonWriter print, void *cookie, const void *obj)
{
    point *p = (point *)obj;
    char tmp[256];

    sprintf(tmp, " /* point */ {\"x\": %d, \"y\": %d}\n", p->x, p->y);
    return print(cookie, tmp, strlen(tmp));
}

void point_new(ZJVal *obj, void *cookie)
{
    zjt_set(obj, cookie);
    printf("MAKE point with: ");
    zj_print(obj);
}

void point_free(ZJVal *obj)
{
    point *p = zjt_get(obj);

    printf("delete point with %d %d\n", p->x, p->y);
}

ZJCustomType T = { print_point, point_new, point_free };


int main(int argc, char **argv)
{
    int i = 1234;
    ZJVal *json = J(
        {
            "hello" : 123,
            "world" : 124
        }
        );

    point p = {
        100,
        200,
    };

    ZJType MyPoint = zjt_register(T);
    ZJVal *pt = zj_new(MyPoint, &p);

    ZJO_SET(json, "pt", pt);
    printf("pt: %p\n", (void *)pt);
    zj_print(pt);
    zj_print(json);
    p.x++;
    p.y += 10;
    zj_print(json);
    zj_delete(json);

    return i;
}
