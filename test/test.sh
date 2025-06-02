#!/bin/bash
source "common.sh"
init_test_state
# 定义要检查的文件夹列表
folders=("log" "hashtable" "shell" "argparse" "file" "string" "tqdm")  # 替换为实际的文件夹名称

# 遍历文件夹并执行其中的 test.sh
for folder in "${folders[@]}"; do
    cd "$folder" || { echo "Error: Failed to enter folder $folder"; exit 1; }

    # 检查是否存在 test.sh
    if [[ -f "test.sh" ]]; then
        split_text "start testing $folder..."
        source test.sh
    else
        ERR "Error: test.sh not found in $folder"
        exit 1
    fi

    # 返回上级目录
    cd ..
    split_text "$folder test completed successfully."
    echo
done

split_text "All tests completed successfully."
show_test_summary