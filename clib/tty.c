/*
 *Copyright (c) 2023 All rights reserved
 *@description: 终端显示设置
 *@author: Zhixing Lu
 *@date: 2023-08-27
 *@email: luzhixing12345@163.com
 *@Github: luzhixing12345
 */

#include "tty.h"

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DEFUALT_LS_COLORS                                                        \
    "rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;" \
    "33;01:or=40;31;01:mi=00:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st="   \
    "37;44:ex=01;32:"

void print_invalid_color_option() {
    printf("Valid arguments are:\n");
    printf("- 'always', 'yes', 'force'\n");
    printf("- 'never', 'no', 'none'\n");
    printf("- 'auto', 'tty', 'if-tty'\n");
}

/**
 * @brief 终端彩色打印
 *
 * @param word
 * @return char* 返回拼接后的字符串
 */
char *colorful_print(term_word *word) {
    static char vt_seq[1024];
    int pos = 0;
    char *p;
    if (!word->word) {
        return NULL;
    }
    if (word->font_type) {
        strcpy(vt_seq + pos, word->font_type);
        pos += strlen(word->font_type);
    }
    if (word->front_color) {
        strcpy(vt_seq + pos, word->front_color);
        pos += strlen(word->front_color);
    }
    if (word->back_color) {
        strcpy(vt_seq + pos, word->back_color);
        pos += strlen(word->back_color);
    }
    if (word->front_rgb) {
        strcpy(vt_seq + pos, word->front_rgb);
        pos += strlen(word->front_rgb);
    }
    if (word->back_rgb) {
        strcpy(vt_seq + pos, word->back_rgb);
        pos += strlen(word->back_rgb);
    }
    strcpy(vt_seq + pos, word->word);
    pos += strlen(word->word);
    p = (char *)TERM_FONT_DEFAULT;
    strcpy(vt_seq + pos, p);
    pos += strlen(p);
    vt_seq[pos] = 0;
    return vt_seq;
}

int check_default_dc_config(char *name, char default_dc_config[18][3]) {
    for (int i = 0; i < 18; i++) {
        // 简化处理
        // TODO
        if (name[0] == '*') {
            return -1;
        }
        if (!strcmp(name, default_dc_config[i])) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 按 : 分割键值对, 按 = 分割 key value
 *
 * @param lscolors_str
 * @param database
 */
void parse_ls_colors(char *lscolors_str, dircolor_database *database) {
    int length = strlen(lscolors_str);
    int item_number = 0;
    for (int i = 0; i < length; i++) {
        if (lscolors_str[i] == ':') {
            item_number++;
        }
    }
    database->dc_kvs = (dc_kv *)malloc(sizeof(dc_kv) * item_number);
    database->item_number = item_number;
    memset(database->dc_kvs, 0, sizeof(dc_kv));
    int p = 0;
    int index = 0;
    char default_dc_config[18][3] = {
        "rs", "di", "ln", "mh", "pi", "so", "do", "bd", "cd", "or", "mi", "su", "sg", "ca", "tw", "ow", "st", "ex"};
    for (int i = 0; i < length; i++) {
        if (lscolors_str[i] == '=') {
            strncpy(database->dc_kvs[index].key, lscolors_str + p, i - p);
            database->dc_kvs[index].key[i - p] = 0;
            p = i + 1;
        } else if (lscolors_str[i] == ':') {
            if (p == i) {
                // 空操作
                p++;
                continue;
            }
            strncpy(database->dc_kvs[index].value, lscolors_str + p, i - p);
            database->dc_kvs[index].value[i - p] = 0;
            p = i + 1;
            int offset = check_default_dc_config(database->dc_kvs[index].key, default_dc_config);
            if (offset != -1) {
                // 结构体内存地址赋值
                // a little tricky
                char *bias = (char *)&(database->dc_kvs[index].value);
                *(u_int64_t *)((char *)database + offset * sizeof(char *)) = (u_int64_t)bias;
            }
            index++;
        }
    }
    // LS_COLORS 没有以 : 结尾, 补齐最后的 value
    if (p != length) {
        strncpy(database->dc_kvs[index].value, lscolors_str + p, length - p);
        database->dc_kvs[index].value[length - p] = 0;
        // printf("%s\n", database->dc_kvs[index].value);
        int offset = check_default_dc_config(database->dc_kvs[index].key, default_dc_config);
        if (offset != -1) {
            // 结构体内存地址赋值
            // a little tricky
            char *bias = (char *)&(database->dc_kvs[index].value);
            *(u_int64_t *)((char *)database + offset * sizeof(char *)) = (u_int64_t)bias;
        }
    }
    // for (int i=0;i<index;i++) {
    //     printf("key = %s, value = %s\n", database->dc_kvs[i].key,
    //     database->dc_kvs[i].value);
    // }
    // printf("rs = %s, di = %s\n", database->rs, database->di);
}

/**
 * @brief 初始化 dircolors 的颜色数据库
 *
 * @param database 需要调用 free_dc_database
 */
void init_dc_database(dircolor_database **database) {
    char *lscolors_str = getenv("LS_COLORS");
    *database = (dircolor_database *)malloc(sizeof(dircolor_database));
    if (!lscolors_str) {
        lscolors_str = (char *)DEFUALT_LS_COLORS;
    }
    parse_ls_colors(lscolors_str, *database);
}

/**
 * @brief 释放 dircolors 数据库
 *
 * @param database
 */
void free_dc_database(dircolor_database *database) {
    free(database->dc_kvs);
    free(database);
}

/**
 * @brief 使用虚拟控制序列打印文件名
 *
 * @param file_name
 * @param full_path
 * @param database
 * @return char*
 */
const char *filename_print(const char *file_name, const char *full_path, dircolor_database *database) {
    if (!database) {
        // database 为 NULL 说明不显示颜色
        return file_name;
    }
    char *color_code = NULL;
    struct stat fs;
    static char result[PATH_MAX];
    memset(result, 0, PATH_MAX);
    // lstat 查看 link 本身, stat 查看 link 指向的 file
    if (lstat(full_path, &fs) == -1) {
        // 文件缺失
        color_code = database->mi;
    } else {
        switch (fs.st_mode & S_IFMT) {
            case S_IFBLK:  // 块设备文件
                color_code = database->bd;
                break;
            case S_IFCHR:  // 字符设备文件
                color_code = database->cd;
                break;
            case S_IFDIR:  // 目录
                if ((fs.st_mode & S_ISVTX)) {
                    if (fs.st_mode & S_IWOTH) {
                        // 其他用户可以写入目录,但仅允许用户删除自己的文件
                        // chmod +t
                        // chmod o+w
                        color_code = database->tw;
                    } else {
                        // chmod +t
                        color_code = database->st;
                    }
                } else if ((fs.st_mode & S_IWOTH)) {
                    // 其他用户可以写入文件
                    // chmod o+w
                    color_code = database->ow;
                } else {
                    color_code = database->di;
                }
                break;
            case S_IFIFO:  // 管道
                color_code = database->pi;
                break;
            case S_IFLNK:  // 符号链接
                if (stat(full_path, &fs) == -1) {
                    // 孤立的符号链接
                    color_code = database->orp;
                } else {
                    // 正常的符号链接
                    color_code = database->ln;
                }
                break;
            case S_IFREG:  // 普通设备
                if (fs.st_mode & S_IXUSR || fs.st_mode & S_IXGRP || fs.st_mode & S_IXOTH) {
                    color_code = database->ex;
                } else if ((fs.st_mode & S_ISUID)) {
                    // 文件拥有者
                    color_code = database->su;
                } else if (fs.st_mode & S_ISGID) {
                    // 文件组
                    color_code = database->sg;
                } else {
                    // 文件名后缀匹配
                    char *dot = strrchr(file_name, '.');
                    int suffix_match = 0;
                    if (dot) {
                        for (int i = 0; i < database->item_number; i++) {
                            if (strlen(database->dc_kvs[i].key) >= 2 && database->dc_kvs[i].key[0] == '*' &&
                                database->dc_kvs[i].key[1] == '.') {
                                // printf("[%s] [%s]\n", database->dc_kvs[i].key + 1, file_name + dot_pos);
                                if (!strcmp(database->dc_kvs[i].key + 1, dot)) {
                                    color_code = database->dc_kvs[i].value;
                                    suffix_match = 1;
                                    break;
                                }
                            }
                        }
                    }
                    if (!suffix_match) {
                        color_code = (char *)"0";
                    }
                }
                break;
            case S_IFSOCK:  // 套接字
                color_code = database->so;
                break;
            default:
                // 未知文件类型
                color_code = database->mi;
                break;
        }
        if (!color_code && fs.st_nlink > 1) {
            color_code = database->mh;
        }
    }
    sprintf(result, "\033[%sm%s%s", color_code, file_name, TERM_FONT_DEFAULT);
    return result;
}
