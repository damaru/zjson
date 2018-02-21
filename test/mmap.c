#include "json.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv) {
	const char* buf;
    char *path = argv[1];
    struct stat sbuf;
    size_t jslen;
    int fd = open(path, O_RDONLY);
	if(stat(path, &sbuf) != 0){
		return -1;
	}
	jslen = sbuf.st_size+10;
	buf = mmap(NULL, jslen,  PROT_READ , MAP_PRIVATE, fd, 0);

    ZJVal *json = zj_parse(buf);
    if(json){

        if(argc == 3 && argv[2]){
		     zj_print(zj_lookup(json, argv[2]));
        } else {
	        //zj_store(json, "json.store");
		}

		zj_print(json);
		zj_delete(json);
    }
}
