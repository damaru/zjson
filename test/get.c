#include <json.h>
#include <stdio.h>
int main(int argc, char **argv)
{
    int i = 1234;
    ZJVal *json = J(
        {
            "workd" : 213.234,
            "hell1" : 124,
            "hell2" : "124",
            "hell3" : 124,
            "list" : [2, 3, 4, 5, 6, 7]
        }
        );
    int j;
    double d;
    char ctr[3209];

    struct foo {
        int i;
        int y;
    } foo;

    const ZJVal *p;
    zj_print(json);
    if (ZJO_GET(json, "workd", &j))
        printf("j: %d\n", j);
    if (ZJO_GET(json, "workd", &d))
        printf("d: %f\n", d);
    ZJO_GET(json, "workd", ctr);
    printf("c: %s\n", ctr);
    ZJO_GET(json, "list", &p);
    zj_print(p);
    ZJO_GET(json, "list", ctr);
    printf("c: %s\n", ctr);
    ZJ_CAST(json, ctr, 1024);
    printf("jsons: %s\n", ctr);
    ZJA_GET(p, 4, &j);
    printf("j: %d\n", j);
    zj_delete(json);
}
