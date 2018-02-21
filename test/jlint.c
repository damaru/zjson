#include <json.h>

int main(int argc, char **argv)
{
	ZJVal *out = zj_load(argv[1]);

	zj_delete(out);
	return 0;
}
