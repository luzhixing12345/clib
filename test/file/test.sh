#!/bin/bash

source "../common.sh"


check_del_dir_1() {
    # check if tmp exists(should not exist)
    if [ -d "tmp" ]; then
        error "tmp still exists"
        return 1
    fi
    return 0
}

mkdir -p tmp
run_and_check "./del_dir tmp" check_del_dir_1

mkdir -p tmp
mkdir -p tmp/a
echo "hello world" > tmp/a/b.txt
mkdir -p tmp/a/b
touch tmp/a/b/c

run_and_check "./del_dir tmp" check_del_dir_1
# check if tmp exists(should not exist)
if [ -d "tmp" ]; then
    echo "tmp exists"
    exit 1
fi
