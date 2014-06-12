all:
	gcc -Wall -pedantic -o bmfs bmfs.c
clean:
	rm -f bmfs
