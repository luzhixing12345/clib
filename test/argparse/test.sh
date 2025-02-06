#!/bin/bash

source "../common.sh"

./base || { ERR "Error: base test failed"; exit 1; }

# 测试用例 1
run_test "./base a.c -o a" \
"file[0] = [a.c]
output file = [a]"

# 测试用例 2
run_test "./base a.c -I include1 -Iinclude2 -I=include3 -o a" \
"file[0] = [a.c]
include path[0] = [include1]
include path[1] = [include2]
include path[2] = [include3]
output file = [a]"

# 测试用例 3
run_test "./base a.c -I include1 -Iinclude2 -I=include3 -labc -l=ccc -o a" \
"file[0] = [a.c]
include path[0] = [include1]
include path[1] = [include2]
include path[2] = [include3]
library name[0] = [abc]
library name[1] = [ccc]
output file = [a]"

run_test "./argparse -v" \
"v0.0.1"

run_test "./argparse -i" \
"[Args Parse Error]: option [--input] needs one argument"

run_test "./argparse -i 100" \
"i = 100"

run_test "./argparse -i 100 -s 200" \
"i = 100
s = 200"

run_test "./argparse -i 100 -s 200 300" \
"i = 100
s = 200
dest = 300"

run_test "./argparse -i 100 -s 200 300 400" \
"i = 100
s = 200
dest = 300
src = 400"

run_test "./argparse -i 100 -s 200 300 \"400\"" \
"i = 100
s = 200
dest = 300
src = 400"

run_test "./argparse -i 100 -s 200 \"300\" \"400\"" \
"i = 100
s = 200
dest = 300
src = 400"

run_test "./argparse -i 100 -s 200 \"300\" \"400\" \"500\"" \
"i = 100
s = 200
dest = 300
src = 400
other number[0] = 500"

run_test "./argparse -i 100 -s 200 \"300\" \"400\" abc" \
"[Args Parse Error]: argument assign to be int but get [abc]"