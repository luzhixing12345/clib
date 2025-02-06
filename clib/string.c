
#include "string.h"
#include <stdio.h>

/**
 * @brief Split a string by a specified delimiter
 * 
 * @param str The input string to be split
 * @param c The delimiter character
 * @param result Pointer to the output array of strings
 * @param length Pointer to the length of the resulting array
 */
void split_str(char *str, char c, char ***result, int *length) {
    if (!str || !result || !length) {
        return; // Invalid input
    }

    int count = 0;
    char *temp = str;
    char *start = NULL;

    // First pass: Count the number of substrings
    while (*temp) {
        if (*temp == c) {
            count++;
        }
        temp++;
    }
    count++; // Account for the last substring after the final delimiter

    // Allocate memory for result
    *result = (char **)malloc(count * sizeof(char *));
    if (!*result) {
        perror("Failed to allocate memory");
        *length = 0;
        return;
    }

    *length = count;
    temp = str;
    count = 0;

    // Second pass: Extract substrings
    while (*temp) {
        if (*temp == c || *temp == '\0') {
            if (start) {
                int substr_len = temp - start;
                (*result)[count] = (char *)malloc((substr_len + 1) * sizeof(char));
                if (!(*result)[count]) {
                    perror("Failed to allocate memory");
                    *length = 0;
                    return;
                }
                strncpy((*result)[count], start, substr_len);
                (*result)[count][substr_len] = '\0'; // Null-terminate the string
                count++;
                start = NULL;
            }
        } else if (!start) {
            start = temp; // Mark the beginning of a substring
        }
        temp++;
    }

    // Handle the last substring
    if (start) {
        int substr_len = temp - start;
        (*result)[count] = (char *)malloc((substr_len + 1) * sizeof(char));
        if (!(*result)[count]) {
            perror("Failed to allocate memory");
            *length = 0;
            return;
        }
        strncpy((*result)[count], start, substr_len);
        (*result)[count][substr_len] = '\0';
    }
}

/**
 * @brief Free the memory allocated for the result array
 * 
 * @param result The array of strings to free
 * @param length The number of strings in the array
 */
void free_split_str(char **result, int length) {
    for (int i = 0; i < length; i++) {
        free(result[i]);
    }
    free(result);
}

/**
 * @brief (原地操作)去除字符串开头结尾的的空格和双引号 ""
 *
 * @param str
 * @return void
 */
void trim(char **str_p) {
    char *new_str;
    char *str = *str_p;
    while (*str == ' ') {
        str++;
    }
    if (*str == '\"') {
        str++;
    }
    int length = (int)strlen(str);
    while (str[length - 1] == ' ') {
        length--;
    }
    if (str[length - 1] == '\"') {
        length--;
    }
    new_str = (char *)malloc(length + 1);
    strncpy(new_str, str, length);
    new_str[length] = '\0';
    free((char *)*str_p);
    *str_p = (char *)new_str;
}

/**
 * @brief 切片
 *
 * @param str
 * @param start 起点index(包含)
 * @param end 终点index(包含), end = -1 表示结尾
 * @return char*(need free)
 */
char *splice(const char *str, int start, int end) {
    if (end == -1) {
        end = (int)strlen(str) - 1;
    }
    char *s = (char *)malloc(sizeof(char) * (end - start + 2));
    strncpy(s, str + start, end - start + 1);
    s[end - start + 1] = '\0';
    return s;
}