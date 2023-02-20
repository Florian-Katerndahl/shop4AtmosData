CC=gcc
WERROR=-Werror
CFLAGS=-Wall -Wextra -Wdouble-promotion -Wuninitialized -Winit-self -flto -O0 -std=c11 -pedantic
LLIBS=-ljansson -lcurl


.PHONY=all clean

all: cams-download cams-process docs

download: src/download.c src/download.h
	$(CC) $(CFLAGS) -c src/download.c -o src/download.o $(LLIBS)

cams-download: cams-download.c download
	$(CC) $(CFLAGS) cams-download.c src/download.o -o cams-download $(LLIBS)

cams-process: cams-process.c
	$(CC) $(CFLAGS) cams-process.c -o cams-process -I/home/florian/Desktop/CAMS

docs: src/download.h
	doxygen Doxyfile

clean:
	rm -rf cams-download
