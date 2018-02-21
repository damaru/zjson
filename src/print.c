#include <string.h>
#include "io.h"
#include "json.h"
#include "structs.h"

static int print_string(ZJsonOut *o, const char *s)
{
	const char *str = s;
	int ret = 0;

	ret += o->write(o, "\"", 1);
	while (*str) {
		switch (*str) {
		case '\"': ret += o->write(o, "\\\"", 2); break;
		case '\\': ret += o->write(o, "\\\\", 2); break;
		case '/': ret += o->write(o, "\\/", 2); break;
		case '\b': ret += o->write(o, "\\b", 2); break;
		case '\f': ret += o->write(o, "\\f", 2); break;
		case '\n': ret += o->write(o, "\\n", 2); break;
		case '\r': ret += o->write(o, "\\r", 2); break;
		case '\t': ret += o->write(o, "\\t", 2); break;
		default:
			ret += o->write(o, str, 1);
		}
		str++;
	}
	ret += o->write(o, "\"", 1);
	return ret;
}

static int serialize_to(const ZJVal *v, int indent, ZJsonOut *o)
{
	char tmp[64];
	int ret = 0;

	if (v->type >= ZJEnd) {
		if (ZJCustomTypes[v->type].serialize)
			return ZJCustomTypes[v->type].serialize((ZJsonWriter)o->write, o, v->string);
	}
	switch (v->type) {
	case ZJTBool:
		ret = v->bval ? o->write(o, "true", 4) : o->write(o, "false", 5);
		break;
	case ZJTNull:
		ret = o->write(o, "null", 4);
		break;
	case ZJTNum:
		ret = o->write(o, v->string, strlen(v->string));
		break;
	case ZJTStr:
		ret = print_string(o, v->string);
		break;
	case ZJTArray:
	{
		int i;
		ret += o->write(o, "[\n", 2);
		indent += 4;
		int ilen = sprintf(tmp, "%*s", indent, " ");

		for (i = 0; i < v->array->count; i++) {
			ret += o->write(o, tmp, ilen);
			ret += serialize_to(v->array->data[i], indent, o);
			if (i < v->array->count - 1)
				ret += o->write(o, ",\n", 2);
			else
				ret += o->write(o, "\n", 1);
		}
		indent -= 4;
		ilen = sprintf(tmp, "%*s", indent, " ");
		ret += o->write(o, tmp, ilen);
		ret += o->write(o, "]", 1);
	}
	break;
	case ZJTObj:
	{
		int i;
		ret += o->write(o, "{\n", 2);
		indent += 4;
		int ilen = sprintf(tmp, "%*s", indent, " ");
		for (i = 0; i < v->object->count; i++) {
			ret += o->write(o, tmp, ilen);
			ret += print_string(o, v->object->pairs[i].key);
			ret += o->write(o, ": ", 2);
			ret += serialize_to(v->object->pairs[i].value, indent, o);
			if (i < v->object->count - 1)
				ret += o->write(o, ",\n", 2);
			else
				ret += o->write(o, "\n", 1);
		}
		indent -= 4;
		ilen = sprintf(tmp, "%*s", indent, " ");
		ret += o->write(o, tmp, ilen);
		ret += o->write(o, "}", 1);
	}
	break;
	default:
		break;
	}
	return ret;
}

int zj_fprint(const ZJVal *v, FILE *fp)
{
	ZJsonOut o = zj_file_writer(fp);

	return serialize_to(v, 0, &o);
}

int zj_sprint(const ZJVal *v, char *str, size_t length)
{
	ZJsonOut o = zj_string_writer(str, length);

	int ret = serialize_to(v, 0, &o);
	str[ret] = 0;
	return ret;
}

int zj_dprint(const ZJVal *v, int fd)
{
	ZJsonOut o = zj_fd_writer(fd);

	return serialize_to(v, 0, &o);
}

int zj_print(const ZJVal *v)
{
	if (v) {
		int ret = zj_fprint(v, stdout);
		ret += printf("\n");
		return ret;
	}
	return 0;
}

int zj_store(const ZJVal *v, const char *path)
{
	FILE *fp = fopen(path, "w");

	if (!fp)
		return 0;
	int ret = zj_fprint(v, fp);
	fclose(fp);
	return ret;
}
