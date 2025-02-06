#!/bin/bash

# 定义 loop_run 函数
loop_run() {
    local max_attempts=$1  # 最大尝试次数
    local command=$2       # 要执行的命令
    local expected_result=$3  # 预期的结果

    # 检查参数是否为空
    if [[ -z "$max_attempts" || -z "$command" || -z "$expected_result" ]]; then
        echo "Error: Missing arguments. Usage: loop_run <max_attempts> <command> <expected_result>"
        return 1
    fi

    # 循环执行命令
    for ((attempt=1; attempt<=max_attempts; attempt++)); do
        echo "Attempt $attempt: Running command '$command'..."

        # 执行命令并捕获返回值
        actual_result=$(eval "$command")

        # 判断结果是否符合预期
        if [[ "$actual_result" -ne "$expected_result" ]]; then
            echo "Error: Attempt $attempt failed. Expected $expected_result, but got $actual_result."
            return 1
        fi
    done
    return 0
}

ERR() {
    echo -e "\033[1;31m$1\033[0m"
}

INFO() {
    echo -e "\033[1;32m$1\033[0m"
}

SPLIT() {
    echo "-----------------------------------------------------------"
}

# 定义测试函数
run_test() {
    local cmd="$1"
    local expected="$2"
    local actual

    # 运行命令并捕获输出
    actual=$(bash -c "$cmd" 2>&1)

    # 比较输出
    if [[ "$actual" == "$expected" ]]; then
        echo "PASS: $cmd"
    else
        echo "FAIL: $cmd"
        ERR "Expected output:"
        ERR "$expected"
        ERR "Actual output:"
        ERR "$actual"
    fi
}
