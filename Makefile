CC=gcc
WERROR=-Werror
CFLAGS=-Wall -Wextra -Wdouble-promotion -Wuninitialized -Winit-self -flto -O0 -std=c11 -pedantic -ggdb #-fsanitize=address -fsanitize=leak -fsanitize=undefined
LLIBS=-ljansson -lcurl
MATH=-lm

.PHONY=all clean

all: cams-download cams-process docs

sort: src/sort.c src/sort.h
	$(CC) $(CFLAGS) -c src/sort.c -o src/sort.o

download: src/download.c src/download.h
	$(CC) $(CFLAGS) -c src/download.c -o src/download.o $(LLIBS) $(MATH)

cams-download: cams-download.c sort download
	$(CC) $(CFLAGS) cams-download.c src/download.o src/sort.o -o cams-download $(LLIBS) $(MATH)

cams-process: cams-process.c
	$(CC) $(CFLAGS) cams-process.c -o cams-process -I/home/florian/Desktop/CAMS

docs: src/download.h
	doxygen Doxyfile

clean:
	rm -f src/sort.o src/download.o
	rm -f cams-download
