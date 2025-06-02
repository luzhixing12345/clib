
workspace=$(pwd)
total_tests=0
passed_tests=0
failed_tests=0

# 定义状态文件来在进程间传递测试计数
TEST_STATE_FILE="/tmp/test_state_$$"

function green() {
    echo -e "\e[32;1m$1\e[0m"
}

function green2() {
    # $1 green and $2 normal
    echo -e "\e[32;1m$1\e[0m $2"
}

function red() {
    echo -e "\e[31;1m$1\e[0m"
}

function yellow() {
    echo -e "\e[33;1m$1\e[0m"
}

function blue() {
    echo -e "\e[34;1m$1\e[0m"
}

function magenta() {
    echo -e "\e[35;1m$1\e[0m"
}

function cyan() {
    echo -e "\e[36;1m$1\e[0m"
}

function purple() {
    echo -e "\e[35;1m$1\e[0m"
}

function info() {
    green "$1"
}

function error() {
    red "$1"
}

# 从状态文件加载计数器
function load_test_counters() {
    if [[ -f "$TEST_STATE_FILE" ]]; then
        source "$TEST_STATE_FILE"
    fi
}

# 保存计数器到状态文件
function save_test_counters() {
    cat > "$TEST_STATE_FILE" << EOF
total_tests=$total_tests
passed_tests=$passed_tests
failed_tests=$failed_tests
EOF
}

function test_add() {
    total_tests=$((total_tests + 1))
}

function test_success() {
    ((passed_tests++))
    save_test_counters
}

function test_fail() {
    ((failed_tests++))
    save_test_counters
}

function run_ok() {
    local cmd="$1"  # 要执行的命令

    if [[ -z "$cmd" ]]; then
        echo "Usage: run_ok <command>"
        return 1
    fi

    load_test_counters
    test_add
    # 运行成功并且正常退出即可
    if eval "$cmd"; then
        green2 "[passed]" "$cmd"
        test_success
        return 0
    else
        error "[failed] Command failed: '$cmd'"
        test_fail
        return 1
    fi
}

# 检查命令执行的输出中是否包含指定的字符串
# 参数:
#   $1: 要执行的命令
#   $2: 要匹配的字符串
# 返回:
#   0: 匹配成功
#   1: 匹配失败
function run_and_match() {
    local cmd="$1"       # 要执行的命令
    local pattern="$2"   # 要匹配的字符串

    if [[ -z "$cmd" || -z "$pattern" ]]; then
        echo "Usage: run_and_match <command> <pattern>"
        return 1
    fi

    load_test_counters

    # 捕获命令输出(stdout + stderr)
    local output
    # purple "Running command: $cmd"
    output=$(eval "$cmd" 2>&1)

    # 打印输出(可选)
    # echo "$output"

    # 检查是否匹配
    test_add
    if echo "$output" | grep -Fq -- "$pattern"; then
        green2 "[passed]" "$cmd"
        test_success
        return 0
    else
        error "[failed] Pattern not found: '$pattern'"
        test_fail
        return 1
    fi
}

# 检查命令执行的输出是否满足，可以指定回调函数处理输出
# 参数:
#   $1: 要执行的命令
#   $2: 用于处理输出的回调函数名
# 返回:
#   0: 回调函数返回成功
#   1: 回调函数返回失败
function run_and_check() {
    local cmd="$1"           # 要运行的命令
    local checker_func="$2"  # 用于处理输出的回调函数名

    if [[ -z "$cmd" || -z "$checker_func" ]]; then
        echo "Usage: run_and_check <command> <checker_function>"
        return 1
    fi

    if ! declare -F "$checker_func" > /dev/null; then
        echo "Error: Checker function '$checker_func' not found."
        return 1
    fi
    
    load_test_counters
    test_add

    # 执行命令,捕获 stdout 和 stderr
    local output
    # purple "[Running command]: $cmd"
    output=$(eval "$cmd" 2>&1)

    # 调用用户提供的 checker 函数,并传入输出内容
    "$checker_func" "$output"
    if [ $? -eq 0 ]; then
        green2 "[passed]" "$cmd"
        test_success
    else
        error "[failed] Checker function '$checker_func' returned failure."
        test_fail
    fi
}


# 定义 loop_run 函数
loop_run() {
    local max_attempts=$1  # 最大尝试次数
    local command=$2       # 要执行的命令
    local expected_result=$3  # 预期的结果

    load_test_counters

    # 检查参数是否为空
    if [[ -z "$max_attempts" || -z "$command" || -z "$expected_result" ]]; then
        echo "Error: Missing arguments. Usage: loop_run <max_attempts> <command> <expected_result>"
        return 1
    fi
    test_add

    # 循环执行命令
    for ((attempt=1; attempt<=max_attempts; attempt++)); do
        echo "Loop $attempt: Running command '$command'"

        # 执行命令并捕获返回值
        actual_result=$(eval "$command")

        # 判断结果是否符合预期
        if [[ "$actual_result" -ne "$expected_result" ]]; then
            echo "Error: Attempt $attempt failed. Expected $expected_result, but got $actual_result."
            test_fail
            return 1
        fi
    done
    test_success
    green2 "[passed]" "'$command' succeeded."
    return 0
}

# split_text "hello world"
# ============================ hello world =============================
function split_text() {
    local text="$1"
    local total_width=70
    local border="${2:-=}"

    local text_len=${#text}
    local padding=$(( (total_width - text_len - 2) / 2 ))

    # 若不是偶数,右边多补一个
    local left_pad=$(printf '%*s' "$padding" '' | tr ' ' "$border")
    local right_pad=$(printf '%*s' "$((total_width - text_len - 2 - padding))" '' | tr ' ' "$border")

    echo -e "${left_pad} $text ${right_pad}"
}

function show_test_summary() {
    load_test_counters

    echo
    split_text "Test Summary"
    echo "Total tests : $total_tests"
    echo "Passed      : $passed_tests"
    echo "Failed      : $failed_tests"
    # if passed + failed != total, then there must be some error
    if [[ $((passed_tests + failed_tests)) -ne $total_tests ]]; then
        error "Some tests failed or were not executed."
    fi
    # if has failed tests, then exit with error code
    if [[ $failed_tests -ne 0 ]]; then
        error "Some tests failed."
        exit 1
    fi
    split_text "End"
    rm -f "$TEST_STATE_FILE"
}

# 初始化测试状态文件
function init_test_state() {
    rm -f "$TEST_STATE_FILE"
    save_test_counters
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
