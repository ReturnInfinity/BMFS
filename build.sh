#!/bin/bash

mkdir bin
gcc -o bin/bmfs src/bmfs.c -Wall -W -pedantic -std=c99
