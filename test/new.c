#include <json.h>
#include <stdio.h>
int main(int argc, char **argv) {
	int i = 1234;
    ZJVal *json = J(
        {
	       "workd": 213,
	       "hell1": 124,
	       "hell2": 124,
	       "hell3": 124,
	       "list": [2, 3,4,5,6,7]
        }
    );
    if(json){
		ZJO_SET(json, "val1", 20);
        zj_print(json);
    }
	ZJVal *obj = zj_object();
	ZJVal *foo = zj_new(ZJNInt, i);
	ZJVal *arr = zj_new(ZJTArray);
	printf("GOT obj %p, foo %p\n",obj,foo);
    zj_print(foo);
	//zj_object_set(obj, "foo", foo);
    zj_print(zj_object_get(json,"list") );
    zj_print(zj_array_get(zj_object_get(json,"list"), 3)  );
    zj_print(zj_new(ZJNInt, 1023));
    zj_print(zj_new(ZJNFloat, 10.23));
    zj_print(zj_new(ZJTStr, "This is a string U:\u1234 10.23"));
    zj_print(zj_new(ZJTNum, "121231231231232352353454565e-19"));
    zj_print(zj_parse("true"));
    zj_print(zj_parse("{}"));
    zj_print(zj_parse("[]"));
	long int j = 12390482039842;
    zj_print(JF("%ld",j));
	ZJO_SET(json, "jello",  20);
	ZJA_SET(arr, 10, foo);
	zj_array_append(arr, ZJNInt, 11);
	zj_array_append(arr, ZJNInt, 12);
	zj_array_append(arr, ZJNInt, 13);
    zj_print(zj_float(1235345));
	//zj_array_set_int(arr, 2, 123456);
	zj_print(arr);
	zj_print(ZJ_NEW(192234));
	zj_print(ZJ_NEW(192.234));
	zj_print(ZJ_NEW("192.234"));
	zj_print(ZJ_NEW(Array));
	zj_print(ZJ_NEW(Object));
	if(!IS_OBJ(j)){ printf("j is not an object\n"); }
	if(IS_OBJ(json)){ printf("json is an object\n"); }
	zj_print(ZJ_NEW(ZJTrue));
	zj_print(ZJ_NEW(ZJFalse));
	ZJO_SET(json, "jelly", "Howdy worlds");
//	zj_print(ZJO_SET(json, "jelly2", 100));
//	zj_print(ZJO_SET(json, "jelly3", 100.234234));
//	zj_print(ZJO_SET(json, "jelly4", J([1,2,3,4,100.234234])));
	zj_print(json);
//	zj_print(ZJO_SET(json, "foo", "Howdy worlds"));
    zj_delete(json);
    return i;
}
