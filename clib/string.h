
#ifndef XSTRING_H
#define XSTRING_H

#include <stdlib.h>
#include <string.h>

/**
 * @brief 分割字符串, 返回数组.
 *
 * @param str 待分割的字符串
 * @param c 分割所使用的字符
 * @param result 返回数组 (call freeSplitStr to free)
 * @param length 数组长度
 */
void split_str(char *str, char c, char ***result, int *length);

void free_split_str(char **result, int length);

/**
 * @brief (原地操作)去除字符串开头结尾的的空格和双引号 ""
 *
 * @param str
 * @return void
 */
void trim(char **str_p);

/**
 * @brief 切片
 *
 * @param str
 * @param start 起点index(包含)
 * @param end 终点index(包含), end = -1 表示结尾
 * @return char*(need free)
 */
char *splice(const char *str, int start, int end);

#endif  // XSTRING_H
