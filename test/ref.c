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
		ZJO_SET(json, "val1", 20);
     	zj_print(json);

		ZJVal *list = zj_ref(zj_object_get(json,"list")); //ref
		const ZJVal *a = zj_lookup(json,"/child/a"); //ptr
		printf("a->ref: %d\n",a->refcount);
		ZJVal *child;
		zj_value(zj_object_get(json,"child") , ZJNRef, &child); //ref
		zj_print(child);

		printf("TYPE: %d\n",zj_type(list));
     	zj_print(list);
		zj_delete(json);
     	//zj_print(json); // should crash
		/*test with valgrind and make sure that the memory for list is freed*/
     	zj_print(list); // ok as we have the ref
		printf("IRET: %d\n", zj_value(zj_array_get(list,2),TYPE(&i),&i));
		printf("%d\n",i);
		printf("IRET2: %d\n", ZJA_GET(list,3,&i));
		printf("%d\n",i);
		printf("RET: %d\n",i);
		zj_delete(list);  // but we need to free it
     	zj_print(a); // ok as we have the ref
		printf("a->ref: %d\n",a->refcount);
		//zj_delete(a);  // compile error
		zj_delete(child);
		//printf("a->ref: %d\n",a->refcount);
     	//zj_print(a); // not ok as child has been deleted
    }
}
