CC=gcc
WERROR=-Werror
CFLAGS=-Wall -Wextra -O0 -std=c11 -pedantic

.PHONY=all clean

all: cams-download cams-process

download: src/download.c
	$(CC) $(CFLAGS) -c src/download.c -o src/download.o -I/home/florian/Desktop/CAMS

cams-download: cams-download.c src/download.c
	$(CC) $(CFLAGS) cams-download.c src/download.o -o cams-download -lcurl -I/home/florian/Desktop/CAMS

cams-process: cams-process.c
	$(CC) $(CFLAGS) cams-process.c -o cams-process

clean:
	rm -rf cams-download