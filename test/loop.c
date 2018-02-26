#include <json.h>
#include <stdio.h>
int main(int argc, char **argv)
{
	int i = 1234;
	ZJVal *json = J(
		{
			"workd" : 213,
			"hell1" : 124,
			"list" : [2, 3, 4, 5, 6, 7, 8, 9, 10],
			"hell2" : 124,
			"hell" : 12,
			"hell3" : 124
		}
		);

	if (json) {
		const char *k;
		ZJVal *v;
		int i = 0;
		const char *key;


		zj_object_foreach(json, ({
						 int __fn__(const char *k, ZJVal * v, void *cookie){
							 printf("K[%d]: %s ", i++, k);
							 zj_print(v);
							 return 0;
						 }; __fn__;
					 }), NULL);

		ZJO_LOOP_BEGIN(json, k, v);
			printf("Key:%s, V:%p\n", k, v);
			if (v->type == ZJTArray) {
				ZJA_LOOP_BEGIN(v, i, lv)
					printf("   index:%d, V:%p\n", i, lv);
				    printf("lv: %d", ZJ_INT(lv));
					if(ZJ_INT(lv) == 8){
						break;
					}
				ZJA_LOOP_END();
			}
		    
			int i = 0;
			ZJ_CAST(v, &i);
			printf("valie: %d\n",i);
			if(i == 12)
					break;
			

		ZJO_LOOP_END();

		zj_delete(json);
	}
}
