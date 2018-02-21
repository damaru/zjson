#include <json.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int i = 1234;
	char strng;
    ZJVal *json = J(
        {
		   	"child": {"a": "AAAAA", "b":"BBBBBB"},
	       	"list": [2, 3,4,5,6,7]
        }
    );
    if(json){
			zj_print(json);
			zj_object_clear(json, "child");
			zj_print(json);
			zj_delete(json);
    }
}
