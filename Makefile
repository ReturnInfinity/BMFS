all:
	gcc -ansi —std=c99 -Wall -pedantic -o bmfs bmfs.c
clean:
	rm -f bmfs
