#include <json.h>
#include <stdio.h>
int main(int argc, char **argv) {
	int i = 1234;
    ZJVal *json = J(
        {
	       "key1": 213,
	       "key2": 124,
	       "key2": 124,
	       "obj": {"a":12, "c":"foobar"},
	       "list": [2, 3,4,5,6,7]
        }
    );
    ZJVal *json2 = J(
        {
	       "key2": null,
	       "obj": {"a":23, "b": "aslkdjalsdk"},
		   "list" : {"foo": 100}
        }
    );


    zj_print(json);
    zj_print(json2);

	ZJVal *j2 = zj_patch(json,  json2);
    zj_print(j2);

	zj_delete(json2);
	zj_delete(j2);

    return i;
}
