#include <json.h>

int main(int argc, char **argv) {
    zj_print(zj_sparsef( JS(
        {
		    "tmp":[ 1, 2, 3, 3 , 45, 56.34234234234253412135346456234236, {"hello":"world", "this":"is cool"}, %d],
            "Hello": "%s",
            "World": %s
        }
    ), 100, "Wwhole", "91827129873192837129387129387192387129837129387129387123981723981723182739182731823" ));

    return 0;
}
