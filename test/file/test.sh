#!/bin/bash

source "../common.sh"

mkdir -p tmp
./del_dir tmp

# check if tmp exists(should not exist)
if [ -d "tmp" ]; then
    echo "tmp exists"
    exit 1
fi

mkdir -p tmp
mkdir -p tmp/a
echo "hello world" > tmp/a/b.txt
mkdir -p tmp/a/b
touch tmp/a/b/c

./del_dir tmp
# check if tmp exists(should not exist)
if [ -d "tmp" ]; then
    echo "tmp exists"
    exit 1
fi
INFO "del_dir test passed"