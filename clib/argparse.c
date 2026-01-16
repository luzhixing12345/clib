/*
 *Copyright (c) 2023 All rights reserved
 *@description: argument parser
 *@author: Zhixing Lu
 *@date: 2023-04-28
 *@email: luzhixing12345@163.com
 *@Github: luzhixing12345
 */

// usage: https://github.com/luzhixing12345/libc

#include "argparse.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief 切片
 *
 * @param str
 * @param start 起点index(包含)
 * @param end 终点index(包含), end = -1 表示结尾
 * @return char*(need free)
 */
static char *splice(const char *str, int start, int end) {
    if (end == -1) {
        end = (int)strlen(str) - 1;
    }
    char *s = (char *)malloc(sizeof(char) * (end - start + 2));
    strncpy(s, str + start, end - start + 1);
    s[end - start + 1] = '\0';
    return s;
}

/**
 * @brief 释放 parser 的内存, 请在程序结束/不再使用 parser 时调用
 *
 * @param parser
 */
void free_argparse(argparse *parser) {
    // fprintf(stderr, "argument parse error, free options\n");
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->value) {
            free(option->value);
        }
        if (__ARGS_NEED_FREE(option->type) && option->match) {
            if (option->type == __ARGPARSE_OPT_STR || option->type == __ARGPARSE_OPT_STR_GROUP) {
                free(*(char **)option->p);
            } else if (option->type == __ARGPARSE_OPT_INTS || option->type == __ARGPARSE_OPT_INTS_GROUP) {
                free(*(int **)option->p);
            } else {
                for (int j = 0; j < option->match; j++) {
                    free((*(char ***)option->p)[j]);
                }
                free(*(char ***)option->p);
            }
        }
    }

    // fprintf(stderr, "finished free options\n");
}

static int check_valid_character(const char *str) {
    int length = (int)strlen(str);
    for (int i = 0; i < length; i++) {
        if (!(islower(str[i]) || str[i] == '_' || str[i] == '-')) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 检验参数合法性
 *
 * @param parser
 */
static void check_valid_options(argparse *parser) {
    // group 不重名
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        // boolean 类型的参数不应该有 append_info
        if (option->type == __ARGPARSE_OPT_BOOLEAN && option->append_info) {
            char *option_name;
            if (option->short_name) {
                option_name = option->short_name;
            } else if (option->long_name) {
                option_name = option->long_name;
            } else if (option->name) {
                option_name = option->name;
            } else {
                option_name = (char *)"unknown";
            }
            fprintf(stderr,
                    "%s: boolean option [%s] should not follow with append info [%s]\n",
                    __ARGS_BUILD_ERROR,
                    option_name,
                    option->append_info);
            exit(FORMAT_ERROR);
        }
        // 检查长参数合法性
        if (option->long_name) {
            if (!strcmp(option->long_name, "--") && !(parser->flag & ARGPARSE_ENABLE_CMD)) {
                fprintf(stderr,
                        "%s: long_name should not be --, add ARGPARSE_ENABLE_CMD flag "
                        "if you want to support -- cmd\n",
                        __ARGS_BUILD_ERROR);
                exit(FORMAT_ERROR);
            }
            char *l_name = splice(option->long_name, 2, -1);
            char *p = l_name;

            // long_name 合法性: a-z-_
            if (check_valid_character(p)) {
                fprintf(stderr, "%s: only [a-z_-] are legal characters instead of [%s]\n", __ARGS_BUILD_ERROR, p);
                free(p);
                exit(FORMAT_ERROR);
            }
            free(p);
        }
    }

    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option1 = &(parser->options[i]);
        for (int j = i + 1; j < parser->args_number; j++) {
            argparse_option *option2 = &(parser->options[j]);
            // 没有两个参数同名
            if (option1->name && option2->name && !strcmp(option1->name, option2->name)) {
                fprintf(stderr, "%s: options have the same name [%s]\n", __ARGS_BUILD_ERROR, option1->name);
                free_argparse(parser);
                exit(FORMAT_ERROR);
            }
            // 没有两个长参数同名
            if (option1->long_name && option2->long_name) {
                if (!strcmp(option1->long_name, option2->long_name)) {
                    fprintf(
                        stderr, "%s: options have the same long_name [%s]\n", __ARGS_BUILD_ERROR, option1->long_name);
                    free_argparse(parser);
                    exit(FORMAT_ERROR);
                }
            }
            // 没有两个短参数同名
            if (option1->short_name && option2->short_name) {
                if (!strcmp(option1->short_name, option2->short_name)) {
                    fprintf(
                        stderr, "%s: options have the same short_name [%s]\n", __ARGS_BUILD_ERROR, option1->short_name);
                    free_argparse(parser);
                    exit(FORMAT_ERROR);
                }
            }
        }
    }

    // if (parser->flag & ARGPARSE_ENABLE_ARG_STICK &&
    //     ((parser->flag & ARGPARSE_ENABLE_EQUAL) || (parser->flag &
    //     ARGPARSE_ENABLE_STICK))) { fprintf(stderr,
    //             "%s: flag collasp for ARGPARSE_ENABLE_EQUAL with
    //             ARGPARSE_ENABLE_EQUAL or " "ARGPARSE_ENABLE_STICK\n",
    //             __ARGS_BUILD_ERROR);
    //     free_argparse(parser);
    //     exit(FORMAT_ERROR);
    // }
}

/**
 * @brief 初始化 argparser
 *
 * @param parser
 * @param options
 * @param flag
 */
void argparse_init(argparse *parser, argparse_option *options, int flag) {
    memset(parser, 0, sizeof(*parser));
    parser->name = NULL;
    parser->description = NULL;
    parser->epilog = NULL;
    parser->options = options;
    parser->args_number = 0;
    parser->flag = flag;
    while (options->type != __ARGPARSE_OPT_END) {
        options++;
        parser->args_number++;
    }
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        option->value = NULL;
        option->match = 0;
        // printf("option short name = [%s], long name = [%s]\n",
        // option->short_name, option->long_name);
    }
    check_valid_options(parser);
    return;
}

/**
 * @brief 添加程序的描述信息
 *
 * @param parser
 * @param name 程序名字
 * @param description 开头的描述信息
 * @param epilog 结尾的描述信息
 */
void argparse_describe(argparse *parser, const char *name, const char *description, const char *epilog) {
    parser->name = name;
    if (description == NULL || (int)strlen(description) == 0) {
        description = "About how to use the program";
    }
    parser->description = description;
    if (epilog == NULL || (int)strlen(epilog) == 0) {
        epilog = "xargparse online help: <https://github.com/luzhixing12345/libc>";
    }
    parser->epilog = epilog;
    return;
}

/**
 * @brief 传递参数
 *
 * @param parser
 * @param option
 */
static void value_pass(argparse *parser, argparse_option *option);

/**
 * @brief 比较参数顺序, 小写字典序
 *
 * @param bind
 * @param b
 * @return int
 */
static int option_cmp(const void *bind, const void *b) {
    argparse_option *p1 = (argparse_option *)bind;
    argparse_option *p2 = (argparse_option *)b;
    if (p1->short_name) {
        if (p2->short_name) {
            char c1 = (p1->short_name)[1];
            char c2 = (p2->short_name)[1];
            char C1 = c1, C2 = c2;
            if (c1 & 0x20) {
                C1 = c1 ^ 0x20;
            }
            if (c2 & 0x20) {
                C2 = c2 ^ 0x20;
            }
            if (C1 == C2) {
                return c1 < c2;
            } else {
                return C1 > C2;
            }
        } else
            return 0;
    }
    return 1;
}

/**
 * @brief 默认的 help 信息输出
 *
 * @param parser
 */
void argparse_info(argparse *parser) {
    int usage_info_length = 0;
    printf("Usage: %s ", parser->name);
    usage_info_length += 7 + strlen(parser->name) + 1;

    int counter = 0;
    char info_array[HELP_INFO_LENGTH];  // 用于下面输出的信息 option_info &
                                        // help_info

    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == __ARGPARSE_OPT_STR_GROUP || option->type == __ARGPARSE_OPT_INT_GROUP) {
            if (option->name) {
                printf("[%s] ", option->name);
                usage_info_length += 3 + strlen(option->name);
            } else {
                printf("[Group] ");
                usage_info_length += 8;
            }
            counter++;
        }
        if (option->type == __ARGPARSE_OPT_STRS_GROUP || option->type == __ARGPARSE_OPT_INTS_GROUP) {
            if (option->name) {
                printf("[%s]... ", option->name);
                usage_info_length += 6 + strlen(option->name);
            } else {
                printf("[Group]... ");
                usage_info_length += 11;
            }
            counter++;
        }
    }
    if (counter != parser->args_number) {
        // 留给 option 的剩余距离
        int remain_option_distance = HELP_INFO_LENGTH - usage_info_length;
        info_array[0] = '[';
        for (int i = 0; i < parser->args_number; i++) {
            argparse_option *option = &(parser->options[i]);
            if (option->type == __ARGPARSE_OPT_INT_GROUP || option->type == __ARGPARSE_OPT_STR_GROUP ||
                option->type == __ARGPARSE_OPT_STRS_GROUP || option->type == __ARGPARSE_OPT_INTS_GROUP) {
                continue;
            }

            char *p = info_array + 1;
            int append_length = 0;
            if (option->append_info) {
                append_length = strlen(option->append_info);
            }
            if (option->short_name) {
                strcpy(p, option->short_name);
                p += 2;
                if (option->append_info) {
                    strcpy(p, option->append_info);
                    p += append_length;
                }
            }
            if (option->long_name) {
                if (option->short_name) {
                    *p++ = ' ';
                }
                strcpy(p, option->long_name);
                p += strlen(option->long_name);
                if (option->append_info) {
                    strcpy(p, option->append_info);
                    p += append_length;
                }
            }
            *p++ = ']';
            *p = 0;
            if (p - info_array <= remain_option_distance) {
                printf("%s", info_array);
                remain_option_distance -= p - info_array;
            } else {
                printf("\n%*s%s", usage_info_length, "", info_array);
                remain_option_distance = HELP_INFO_LENGTH - usage_info_length - (p - info_array);
            }
        }
    }
    printf("\n");
    if (parser->description) {
        printf("\nDescription: %s\n", parser->description);
    }
    printf("\n");

    /*
    -s            --long-name       help information...
    -a <text>     --append <text>   Sentences that are too
                                    long will be automatically
                                    wrapped without breaking
                                    the words.
    -h            --help

  ◄──►         ◄───►               ◄─►

  left_space   mid_space           right_space

  ◄─────────────────────────────────────────────────────────────►

                      total_space
  */

    int short_append_align = 0;     // 短参数的补充信息的对齐长度
    int long_opt_append_align = 0;  // 长参数+补充信息的对齐长度
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->short_name && option->append_info) {
            int append_length = (int)strlen(option->append_info);
            short_append_align = MAX(short_append_align, append_length);
        }
        if (option->long_name) {
            int option_length = (int)strlen(option->long_name);
            if (option->append_info) {
                option_length += (int)strlen(option->append_info);
            }
            long_opt_append_align = MAX(long_opt_append_align, option_length);
        }
    }

    if ((parser->flag & ARGPARSE_SORT)) {
        qsort(parser->options, (size_t)parser->args_number, sizeof(argparse_option), option_cmp);
    }

    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == __ARGPARSE_OPT_INT_GROUP || option->type == __ARGPARSE_OPT_STR_GROUP ||
            option->type == __ARGPARSE_OPT_STRS_GROUP || option->type == __ARGPARSE_OPT_INTS_GROUP) {
            continue;
        }

        // 短参数部分
        printf("%*s", HELP_INFO_LEFT_SPACE, "");
        if (option->short_name) {
            printf("%s", option->short_name);
            printf("%-*s", short_append_align, option->append_info ? option->append_info : "");
        } else {
            printf("  %*s", short_append_align, "");
        }
        printf("%*s", HELP_INFO_MID_SPACE, "");

        // 长参数部分
        int mid_rest_space = long_opt_append_align;
        if (option->long_name) {
            printf("%s", option->long_name);
            mid_rest_space -= (int)strlen(option->long_name);
            if (option->append_info) {
                printf("%s", option->append_info);
                mid_rest_space -= (int)strlen(option->append_info);
            }
        }
        printf("%*s", mid_rest_space, "");
        printf("%*s", HELP_INFO_RIGHT_SPACE, "");

        // 帮助信息部分
        if (option->help_info) {
            int help_info_length = (int)strlen(option->help_info);
            int help_info_space = HELP_INFO_LENGTH - HELP_INFO_LEFT_SPACE - 2 - short_append_align -
                                  HELP_INFO_MID_SPACE - long_opt_append_align - HELP_INFO_RIGHT_SPACE;
            if (help_info_length > help_info_space) {
                int p = 0;
                while (help_info_length > help_info_space) {
                    strncpy(info_array, option->help_info + p, (size_t)help_info_space);
                    info_array[help_info_space] = 0;
                    p += help_info_space;
                    printf("%s\n%*s",
                           info_array,
                           HELP_INFO_LEFT_SPACE + 2 + short_append_align + HELP_INFO_MID_SPACE + long_opt_append_align +
                               HELP_INFO_RIGHT_SPACE,
                           "");
                    help_info_length -= help_info_space;
                }
                printf("%s", option->help_info + p);
            } else {
                printf("%s", option->help_info);
            }
        }
        printf("\n");
    }
    if (parser->epilog) {
        printf("\n%s\n", parser->epilog);
    }
}

/**
 * @brief 判断长参数是否匹配
 *
 * @param parser
 * @param str
 * @return argparse_option 返回匹配的option, 否则返回 NULL
 */
static argparse_option *check_argparse_loptions(argparse *parser, const char *str) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == __ARGPARSE_OPT_INT_GROUP || option->type == __ARGPARSE_OPT_STR_GROUP ||
            option->type == __ARGPARSE_OPT_INTS_GROUP || option->type == __ARGPARSE_OPT_STRS_GROUP) {
            continue;
        }
        if (option->long_name && !strcmp(option->long_name, str)) {
            return option;
        }
    }
    return NULL;
}

/**
 * @brief 判断短参数是否匹配, 并修改 option->match
 *
 * @param parser
 * @param str
 * @return argparse_option* 返回匹配的option, 否则返回 NULL
 */
static argparse_option *check_argparse_soptions(argparse *parser, const char *str) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == __ARGPARSE_OPT_INT_GROUP || option->type == __ARGPARSE_OPT_STR_GROUP ||
            option->type == __ARGPARSE_OPT_INTS_GROUP || option->type == __ARGPARSE_OPT_STRS_GROUP) {
            continue;
        }
        if (option->short_name && !strcmp(option->short_name, str)) {
            // printf("matched %s\n", option->short_name);
            return option;
        }
    }
    return NULL;
}

/**
 * @brief 解析组, 修改match和value
 *
 * @param parser
 * @param str
 * @return int 如果已经没有可解析的组则出现错误,返回1
 */
static int check_argparse_groups(argparse *parser, const char *str, int *match_pos) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);

        if (option->type == __ARGPARSE_OPT_INT_GROUP || option->type == __ARGPARSE_OPT_STR_GROUP) {
            if (!option->match) {
                if (option->value) {
                    free(option->value);
                }
                option->value = (char *)malloc(sizeof(char) * ((int)strlen(str) + 1));
                option->pos = *match_pos++;
                strcpy(option->value, str);
                // printf("matched [%s] for group [%s]\n", option->value, option->name);
                value_pass(parser, option);
                return 0;
            }
        }
        if (option->type == __ARGPARSE_OPT_INTS_GROUP || option->type == __ARGPARSE_OPT_STRS_GROUP) {
            if (option->value) {
                free(option->value);
            }
            option->value = (char *)malloc(sizeof(char) * ((int)strlen(str) + 1));
            option->pos = *match_pos++;
            strcpy(option->value, str);
            // printf("[%d]: matched [%s] for group [%s]\n", __LINE__, option->value,
            // option->name);
            value_pass(parser, option);
            return 0;
        }
    }
    return 1;
}

/**
 * @brief 传递解析后的参数
 *
 * @param option
 * @param enable_multi 是否允许多个分离参数
 */
static void value_pass(argparse *parser, argparse_option *option) {
    // 警告信息, 覆盖
    // if (option->match && (option->type == ARGPARSE_OPT_STR || option->type ==
    // ARGPARSE_OPT_INT) &&
    //     !(parser->flag & ARGPARSE_IGNORE_WARNING)) {
    //     fprintf(stderr, "%s: ignore argument [%s]\n", __ARGS_PARSE_WARNING,
    //     option->value); return;
    // }

    // 单个匹配
    if ((option->type == __ARGPARSE_OPT_STR) || option->type == __ARGPARSE_OPT_STR_GROUP) {
        if (option->match >= 1) {
            free(*(char **)option->p);
        }
        *(char **)option->p = (char *)malloc((int)strlen(option->value) + 1);
        strcpy(*(char **)option->p, option->value);
        option->match = 1;
        return;
    } else if ((option->type == __ARGPARSE_OPT_INT) || option->type == __ARGPARSE_OPT_INT_GROUP) {
        int value = 0;
        int signal = 1;
        char *temp = option->value;
        if (*temp == '-') {
            signal = -1;
            temp++;
        }
        while (*temp != '\0') {
            if (*temp < '0' || *temp > '9') {
                fprintf(stderr, "%s: argument assign to be int but get [%s]\n", __ARGS_PARSE_ERROR, option->value);
                free_argparse(parser);
                exit(FORMAT_ERROR);
            }
            value = value * 10 + (*temp) - '0';
            temp++;
        }
        *(int *)option->p = value * signal;
        option->match = 1;
        return;
    }
    // 多个匹配的情况
    // -D __GNU__ -D __KERNEL
    int match_number = ++option->match;
    if (option->type == __ARGPARSE_OPT_STRS || option->type == __ARGPARSE_OPT_STRS_GROUP) {
        char **new_p;
        if (match_number == 1) {
            new_p = (char **)realloc(NULL, sizeof(char *) * match_number);
        } else {
            new_p = (char **)realloc(*(char ***)option->p, sizeof(char *) * match_number);
        }
        *(char ***)option->p = new_p;
        char *value_str = splice(option->value, 0, -1);
        (*(char ***)option->p)[match_number - 1] = value_str;

    } else if (option->type == __ARGPARSE_OPT_INTS || option->type == __ARGPARSE_OPT_INTS_GROUP) {
        int value = 0;
        int signal = 1;
        char *temp = option->value;
        if (*temp == '-') {
            signal = -1;
            temp++;
        }
        while (*temp != '\0') {
            if (*temp < '0' || *temp > '9') {
                fprintf(stderr, "%s: argument assign to be int but get [%s]\n", __ARGS_PARSE_ERROR, option->value);
                free_argparse(parser);
                exit(FORMAT_ERROR);
            }
            value = value * 10 + (*temp) - '0';
            temp++;
        }
        int *new_p;
        if (match_number == 1) {
            new_p = (int *)realloc(NULL, sizeof(int) * match_number);
        } else {
            new_p = (int *)realloc(*(int **)option->p, sizeof(int) * match_number);
        }
        (*(int **)option->p) = new_p;
        (*(int **)option->p)[match_number - 1] = value * signal;
    } else {
        fprintf(stderr, "%s: unknown option type for [%s]\n", __ARGS_PARSE_ERROR, option->name);
        free_argparse(parser);
        exit(FORMAT_ERROR);
    }
}

/**
 * @brief 解析命令行参数
 *
 * @param parser
 * @param argc
 * @param argv
 */
void argparse_parse(argparse *parser, int argc, char **argv) {
    // 不使用 i 作为匹配位置的索引, 因为需要考虑粘连参数的先后顺序 -abc
    int match_pos = 1;
    for (int i = 1; i < argc; i++) {
        int argv_length = (int)strlen(argv[i]);
        if (argv_length >= 2 && argv[i][0] == '-') {
            // --long_name
            argparse_option *option;
            if (argv[i][1] == '-') {
                option = check_argparse_loptions(parser, argv[i]);
            } else {
                option = check_argparse_soptions(parser, argv[i]);
            }

            if (option == NULL) {
                if (parser->flag & ARGPARSE_ENABLE_EQUAL) {
                    int equal_pos = strchr(argv[i], '=') ? strchr(argv[i], '=') - argv[i] : -1;
                    if (equal_pos != -1) {
                        char *name = splice(argv[i], 0, equal_pos - 1);
                        if ((int)strlen(name) >= 2 && name[0] == '-') {
                            if (name[1] == '-') {
                                option = check_argparse_loptions(parser, name);
                            } else {
                                option = check_argparse_soptions(parser, name);
                            }
                        }

                        free(name);
                        if (option) {
                            if (option->value) {
                                free(option->value);
                            }
                            char *value = splice(argv[i], equal_pos + 1, -1);
                            option->value = value;
                            option->pos = match_pos++;
                            value_pass(parser, option);
                            continue;
                        }
                    }
                }
                if (parser->flag & ARGPARSE_ENABLE_STICK) {
                    char *short_name = splice(argv[i], 0, 1);
                    option = check_argparse_soptions(parser, short_name);
                    free(short_name);
                    if (option) {
                        // 只判断非 boolean 类型的粘连情况
                        if (option->type != __ARGPARSE_OPT_BOOLEAN) {
                            if (option->value) {
                                free(option->value);
                            }
                            char *value = splice(argv[i], 2, -1);
                            option->value = value;
                            option->pos = match_pos++;
                            value_pass(parser, option);
                            continue;
                        }
                        // if (option->type == ARGPARSE_OPT_BOOLEAN && !(parser->flag &
                        // ARGPARSE_IGNORE_WARNING)) {
                        //     fprintf(stderr,
                        //             "%s: Boolean argument [%s] is sticky in [%s], do you
                        //             mean " "ARGPARSE_ENABLE_ARG_STICK?\n",
                        //             __ARGS_PARSE_ERROR,
                        //             option->short_name,
                        //             argv[i]);
                        //     free_argparse(parser);
                        //     exit(FORMAT_ERROR);
                        // }
                    }
                }
                if ((parser->flag & ARGPARSE_ENABLE_ARG_STICK) && argv[i][1] != '-') {
                    char s[3] = {'-', '0', '\0'};
                    int n = (int)strlen(argv[i]);
                    for (int j = 1; j < n; j++) {
                        s[1] = argv[i][j];
                        option = check_argparse_soptions(parser, s);
                        if (option == NULL) {
                            fprintf(stderr, "%s: no match options for [%s]\n", __ARGS_PARSE_ERROR, argv[i]);
                            free_argparse(parser);
                            exit(FORMAT_ERROR);
                        } else {
                            if (option->type != __ARGPARSE_OPT_BOOLEAN) {
                                fprintf(stderr,
                                        "%s: only boolean type should be sticky for [%c] in "
                                        "[%s]\n",
                                        __ARGS_PARSE_ERROR,
                                        argv[i][j],
                                        argv[i]);
                                free_argparse(parser);
                                exit(FORMAT_ERROR);
                            } else {
                                option->match = 1;
                                option->pos = match_pos++;
                                if (option->p) {
                                    *(int *)option->p = 1;
                                }
                            }
                        }
                    }
                    continue;
                }
                if (!(parser->flag & ARGPARSE_IGNORE_UNKNOWN)) {
                    fprintf(stderr, "%s: no match options for [%s]\n", __ARGS_PARSE_ERROR, argv[i]);
                    free_argparse(parser);
                    exit(FORMAT_ERROR);
                }

            } else {
                if (option->type == __ARGPARSE_OPT_BOOLEAN) {
                    option->match = 1;
                    option->pos = match_pos++;
                    if (option->p) {
                        *(int *)option->p = 1;
                    }
                    continue;
                }
                // 正确解析, 读取下一个参数
                if (i == argc - 1) {
                    fprintf(stderr, "%s: option [%s] needs one argument\n", __ARGS_PARSE_ERROR, option->long_name);
                    free_argparse(parser);
                    exit(FORMAT_ERROR);
                }
                // if (argv[i + 1][0] == '-' && !(parser->flag &
                // ARGPARSE_IGNORE_WARNING)) {
                //     fprintf(stderr,
                //             "%s: [%s] will be passed as the argument for [%s]\n",
                //             __ARGS_PARSE_WARNING,
                //             argv[i + 1],
                //             argv[i]);
                // }
                if (option->value) {
                    free(option->value);
                }
                if ((parser->flag & ARGPARSE_ENABLE_CMD) && option->long_name && !strcmp(option->long_name, "--")) {
                    // 如果匹配到 -- 那么停止解析, 把后面所有的参数都赋给 option->value
                    int total_len = 0;
                    for (int j = i + 1; j < argc; j++) {
                        total_len += (int)strlen(argv[j]) + 1;
                    }
                    option->value = (char *)malloc(sizeof(char) * total_len);
                    memset(option->value, 0, sizeof(char) * total_len);
                    option->pos = match_pos++;
                    for (int j = i + 1; j < argc; j++) {
                        strcat(option->value, argv[j]);
                        if (j != argc - 1) {
                            strcat(option->value, " ");
                        }
                    }
                    value_pass(parser, option);
                    break;
                } else {
                    option->value = (char *)malloc(sizeof(char) * ((int)strlen(argv[i + 1]) + 1));
                    strcpy(option->value, argv[i + 1]);
                    option->pos = match_pos++;
                    value_pass(parser, option);
                    // printf("matched [%s]:[%s]\n", option->long_name, argv[i + 1]);
                    i++;
                }
            }
            continue;
        }

        // 当作正常参数传入
        if (check_argparse_groups(parser, argv[i], &match_pos)) {
            if (!(parser->flag & ARGPARSE_IGNORE_UNKNOWN)) {
                fprintf(stderr, "%s: [%s] doesn't match any left group\n", __ARGS_PARSE_WARNING, argv[i]);
            }
        }
    }

    // MUST 检查必传参数
    // for (int i = 0; i < parser->args_number; i++) {
    //     argparse_option *option = &(parser->options[i]);
    //     if (option->must) {
    //         if (!option->match) {
    //             fprintf(stderr, "Error: argument option [%s] is must\n",
    //             option->name); free_argparse(parser);
    //             exit(FORMAT_ERROR);
    //         }
    //     }
    // }
}

/**
 * @brief 参数是否匹配
 *
 * @param parser
 * @param name
 * @return int 如果未匹配返回0; 如果匹配,返回值为匹配的个数
 */
int arg_ismatch(argparse *parser, const char *name) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->name && !strcmp(option->name, name)) {
            return option->match;
        }
    }
    fprintf(stderr, "%s: no matched name in options for [%s]\n", __ARGS_PARSE_WARNING, name);
    return 0;
}

/**
 * @brief 查找参数匹配的位置, 从 1 起

   ./main 100 -i 200
          |    | |
          1    2 3

   ./main -abc -d 100
           |||  | |
           123  4 5
 *
 * @param parser
 * @param name
 * @return int
 */
int arg_match_pos(argparse *parser, char *name) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->name && !strcmp(option->name, name)) {
            return option->pos;
        }
    }
    fprintf(stderr, "%s: no matched name in options for [%s]\n", __ARGS_PARSE_WARNING, name);
    return 0;
}