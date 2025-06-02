#!/bin/bash
source ../common.sh

# run 10 times, thread_log use 1000 threads to log, expect 1001 lines
loop_run 10 "./thread_log | grep INFO | wc -l" 1001

check_log_file() {
    # check if log_file.txt exists
    if [ -f "log_file.txt" ]; then
        # check if log_file.txt contains "hello world"
        if grep -q "hello world" log_file.txt; then
            return 0
        else
            error "log_file.txt does not contain 'hello world'"
        fi
    else
        error "log_file.txt not found"
    fi
    return 1
}

run_and_check "./log_file" check_log_file

check_log_level() {
    output=$1
    # check if output contains "this is debug"
    has_debug=$(echo $output | grep -q "this is debug" | wc -l)
    if [ $has_debug -eq 0 ]; then
        return 0
    else
        error "log level debug should not be printed"
    fi
    return 1
}

run_and_check "./log_level" check_log_level

# has_debug=$(./log_level | grep -q "this is debug" | wc -l)
# should not have debug log
