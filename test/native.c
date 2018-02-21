#include <stdio.h>
#include <stdio.h>
#include <json.h>

#define TYPE(obj) (_Generic((obj),\
						    bool: "bool", \
						    int: "int", \
						    char: "char", \
						    const char *: "string", \
						    double: "double",\
						    float: "float",\
						    ZJVal*: "ref"))

#define PTYPE(obj) (_Generic((obj),\
				    bool*: "bool", \
				    int*: "int", \
				    char *: "string", \
				    double*: "double",\
				    ZJVal**: "ref")


void  main(){
		int i;
		typedef int k;
		k kv;
		float f;
		double d;
		char c;
		char str[32];
		char *str2;
		const char *str3;
		printf("TYPE: %s\n", TYPE(i) );
		printf("TYPE: %s\n", TYPE(kv) );
		printf("TYPE: %s\n", TYPE(f) );
		printf("TYPE: %s\n", TYPE(d) );
		printf("TYPE: %s\n", TYPE(c) );
		printf("TYPE: %s\n", TYPE(str) );
		printf("TYPE: %s\n", TYPE(str2) );
		printf("TYPE: %s\n", TYPE(str3) );
}
