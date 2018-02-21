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
			const char *k;
			ZJVal *v;
			int i = 0;
			zj_object_foreach(json, ({
				int __fn__(const char *k, ZJVal *v){
					printf("K[%d]: %s ",i++, k);
					zj_print(v);
					return 0;
				}; __fn__;}));

			ZJO_LOOP_BEGIN(json, k, v);
				printf("Key:%s, V:%p\n", k, v );
				if(v->type == ZJTArray){
					ZJA_LOOP_BEGIN(v, i, lv)
						printf("   index:%d, V:%p\n", i, lv );
					ZJA_LOOP_END();
				}
			ZJO_LOOP_END();

			zj_delete(json);
	}
}
