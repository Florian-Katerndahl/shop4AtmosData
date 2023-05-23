CC=gcc
WERROR=-Werror
CFLAGS=-Wall -Wextra -Wdouble-promotion -Wuninitialized -Winit-self -flto -O0 -std=c11 -pedantic -fsanitize=address -fsanitize=leak -fsanitize=undefined #-ggdb
LLIBS=-ljansson -lcurl
ECCODES=-leccodes
MATH=-lm

.PHONY=all clean

all: cams-download cams-process docs

sort: src/sort.c src/sort.h
	$(CC) $(CFLAGS) -c src/sort.c -o src/sort.o

download: src/download.c src/download.h
	$(CC) $(CFLAGS) -c src/download.c -o src/download.o $(LLIBS) $(MATH)

api: src/api.c src/api.h
	$(CC) $(CFLAGS) -c src/api.c -o src/api.o $(LLIBS) $(MATH)

gributils: src/gributils.c src/gributils.h
	$(CC) $(CFLAGS) -c src/gributils.c -o src/gributils.o $(LLIBS) $(ECCODES) $(MATH)

cams-download: cams-download.c sort download api
	$(CC) $(CFLAGS) cams-download.c src/download.o src/sort.o src/api.o -o cams-download $(LLIBS) $(MATH)

cams-process: cams-process.c gributils
	$(CC) $(CFLAGS) cams-process.c src/gributils.o -o cams-process

docs: src/download.h src/sort.h src/api.h src/gributils.h
	doxygen Doxyfile

clean:
	rm -f src/sort.o src/download.o src/api.o src/gributils.o
	rm -f cams-download cams-process
	rm -rf docs
