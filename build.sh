#!/bin/bash

mkdir -p bin
gcc -o bin/bmfs src/bmfs.c -Wall -W -pedantic -std=c99
