

OBJS= build/obj/json.o build/obj/print.o build/obj/parser.o build/obj/io.o build/obj/builder.o build/obj/pointer.o
TESTS=build/test/ref build/test/parsef build/test/test build/test/jcat build/test/new build/test/custom build/test/patch build/test/jlint build/test/mmap build/test/get build/test/loop\
	  build/test/clear

all: build/lib/libzjson.so build/lib/libzjson.a $(TESTS) 

#OPTS=-g
OPTS=-O3

CFLAGS=-Iinclude -pedantic-errors -Wall -Werror

build/lib/libzjson.so: $(OBJS)
		gcc $(OPTS) -fPIC $(CFLAGS) --shared $< -o $@

build/lib/libzjson.a: $(OBJS)
	ar rs $@ $^
	ranlib $@

build/obj/%.o:src/%.c include/json.h  src/*.h
		gcc $(OPTS) -fPIC $(CFLAGS) -c $< -o $@

build/test/%:test/%.c build/lib/libzjson.a
	gcc $(OPTS) -Iinclude $^ -o $@

clean:
	rm -v $(OBJS) $(TESTS) build/lib/libzjson.so build/lib/libzjson.a 
