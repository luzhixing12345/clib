
#include "shell.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parseline(const char *cmdbuf, struct cmd_arg *args) {
    if (!cmdbuf || !args) {
        return -1;  // 参数无效
    }

    // 复制输入字符串到可修改的缓冲区
    args->_cmdbuf = strdup(cmdbuf);
    char *buf = args->_cmdbuf;
    if (!buf) {
        return -1;  // 内存分配失败
    }

    // 动态分配 argv 数组
    args->argv = malloc(sizeof(char *) * (strlen(buf) / 2 + 1));  // 最多 cmdbuf_len/2 个参数
    if (!args->argv) {
        free(buf);
        return -1;  // 内存分配失败
    }

    args->argc = 0;
    char *p = buf;
    while (*p) {
        // 跳过前导空格
        while (isspace((unsigned char)*p)) {
            p++;
        }

        // 如果到达字符串末尾,停止解析
        if (*p == '\0') {
            break;
        }

        char *start;
        if (*p == '"') {  // 如果以双引号开头,处理引号内部内容
            p++;
            start = p;  // 引号内部起点
            while (*p && *p != '"') {
                p++;
            }
            if (*p == '"') {  // 找到结束引号
                *p = '\0';
                p++;
            }
        } else {  // 普通参数(以空格分隔)
            start = p;
            while (*p && !isspace((unsigned char)*p)) {
                p++;
            }
            if (*p) {  // 用 '\0' 标记参数末尾
                *p = '\0';
                p++;
            }
        }

        // 存储参数
        args->argv[args->argc++] = start;
    }

    args->argv[args->argc] = NULL;  // 以 NULL 结束参数数组
    return 0;                       // 成功解析
}

void free_cmd_arg(struct cmd_arg *args) {
    if (args && args->argv) {
        free(args->argv);
        free(args->_cmdbuf);
        args->argv = NULL;
    }
}