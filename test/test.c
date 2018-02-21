#include <json.h>
#include <stdio.h>


#if 0
int main1(int argc, char **argv){
    ZJVal *arr[6] = {
            zj_int(1),
            zj_bool(true),
            zj_int(2),
            //zj_null(),
            zj_number("3029384029384203489"),
            //zj_bool(false),
            zj_int(4),
            zj_string("last"),
    };

    ZJVal *a = zj_array();
    zj_array_set(a, 0, arr[0]);
    zj_array_set(a, 1, arr[1]);
    zj_array_set(a, 2, arr[2]);
    zj_array_set(a, 3, arr[3]);
    zj_array_set(a, 4, arr[4]);
    zj_array_set(a, 5, arr[5]);
    zj_print(a);
    zj_array_set(a, 4, zj_string("new"));
    zj_array_set(a, 18, zj_array_get(a, 4));
    zj_print(a);
    ZJVal *o = zj_object();
    zj_object_set(o, "c", zj_int(1));
    zj_object_set(o, "a", zj_string("aval"));
    zj_object_set(o, "b", zj_float(42.42));
    zj_object_set(o, "arr", a);
    arr[0] = zj_int(1);
    arr[1] = zj_int(2);
    arr[2] = zj_object();
    a  = zj_array();
    ZJVal *o2 = zj_object();
    zj_array_set(a, 0, arr[0]);
    zj_array_set(a, 1, arr[1]);
    zj_object_set(o2, "a", a);
    zj_object_set(o2, "b", a);
    zj_object_set(o2, "c", a);
    zj_object_set(o, "c", o2);
    zj_object_set(o, "c2", o2);
    zj_object_set_int(o, "int", 1234);
    zj_object_set_float(o, "float", 12.34);
    zj_object_set_float(o, "float2", 12.34);
    zj_object_set_float(arr[2], "float3", 12.34);
    zj_object_set_float(o, "float4", 12.34);
    zj_object_set_string(o, "str", "Hello World");
    zj_object_set_string(o, "", "Hello World");
    zj_object_set_number(o, "str2", "123456");
    zj_object_set(o, "tmp", arr[2]);

    zj_print(o);
    zj_print(zj_object_get(o, "str"));
    zj_print(zj_object_get(o, "str2"));
    zj_delete(o);
    //printf("objsize: %ld bytes\n", sizeof(ZJVal));

    ZJVal *p = zj_string("Hello World");
    zj_print(p);
    zj_delete(p);

    const char *js = "[-012323e-23, null, 2, {\"a\":23, \"kjas\":[3,4,5]}, false, 3, true, 4, \"hello world !\", {\"foo\": 122312293847298347293874293487293847293847293487239472933 }]";
    zj_print(zen_jsonparse_str(js));
    const char *js2 = "{\"a\":23, \"kjas\":[3,4,5], \"fop\" :  \"aslkd aklsdjal ;skdl ;kasdl;ak dl;akd;laksdakd;ak das;\"}";
    zj_print(zen_jsonparse_str(js2));
    const char *js3 = "  {\"alkjsdlakj\":23,\"4\":null,} ";
    zj_print(zen_jsonparse_str(js3));
    const char *js4 = "  [-0.3e2] ";
    zj_print(zen_jsonparse_str("[\n-]"));
    zj_print(zen_jsonparse_str(js4));
    return 0;
}
#endif

int main(int argc, char **argv) {
    ZJVal *json = zj_load(argv[1]);
    if(json){
        char s[1024];
        zj_sprint(json,  s, 1024);
        printf("serial: [[%s]]\n", s);
    }
    zj_delete(json);
    //zj_print(zen_jsonparse_stringf("[%d,%d, %d,\"%s\",%s]", 10, 100, 200000, "2032983472938742893472893742893742987498273489", "{\"foo\":918273917239187298127319827319827319827391827391283}"), 0);
    //zj_print(zen_jsonparse_stringf("[%d,%d,\"%s\"]",10,10,"fooooooooooooooooooooooo"), 0);
    zj_print(JF( JS(
        {
            "Hello": "%s",
            "World": %s
        }
    ), "Wwhole", "91827129873192837129387129387192387129837129387129387123981723981723182739182731823" ));

    json = JF(JS([
            1, 2, 3, 3, 45, 56.34234234234253412135346456234236, {"hello":"world", "this":"is cool"},
            %d
        ]), 100);
    zj_print(json);
    printf("getting int: %d\n",zj_array_get_int(json, 5));
    printf("getting float: %f\n",zj_array_get_double(json, 5));
    printf("getting string: %s\n",zj_array_get_string(json, 5));

    //printf("[%d,%d, %d,\"%s\",%s]\n", 10, 100, 200000, "2032983472938742893472893742893742987498273489", "{\"foo\":918273917239187298127319827319827319827391827391283}");
    return 0;
}
