#!/bin/bash

function clean_dir {
	cd "$1"
	./clean.sh
	cd ".."
}

clean_dir "lib"
clean_dir "utils"
