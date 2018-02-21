#ifndef _JSON_IO_H
#define _JSON_IO_H

#include <stdio.h>

typedef struct ZJsonIn ZJsonIn;
typedef struct ZJsonOut ZJsonOut;

struct ZJsonIn {
	char fin[64];
	union {
		const char *in;
		FILE *fp;
		int	fd;
	};

	size_t	length;
	size_t	count;
	char	c;
	char	next;
	int	(*read)(ZJsonIn *);
};

struct ZJsonOut {
	char fout[64];
	union {
		struct {
			size_t	length;
			size_t	count;
			char *	out;
		};
		FILE *	fp;
		int	fd;
	};

	int (*write)(ZJsonOut *, const char *, size_t);
};

ZJsonIn zj_fd_reader(int fd);
ZJsonIn zj_file_reader(FILE *fp);
ZJsonIn zj_string_reader(const char *js, size_t count);

ZJsonOut zj_fd_writer(int fd);
ZJsonOut zj_file_writer(FILE *fp);
ZJsonOut zj_string_writer(char *str, size_t length);

#endif
