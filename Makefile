all: bmfs

bmfs: src/bmfs.c
	$(CC) -o bmfs src/bmfs.c -Wall -W -pedantic -std=c99

clean:
	rm -f bmfs

