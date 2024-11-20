#!/bin/bash

mkdir -p bin
gcc -o bin/bmfs src/bmfs.c -Wall -W -pedantic -std=c99
gcc -o bin/bmfslite src/bmfslite.c -Wall -W -pedantic -std=c99
