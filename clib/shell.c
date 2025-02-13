
#include "shell.h"

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "file.h"
#include "keyboard.h"
#include "log.h"
#include "tty.h"
#include "utils.h"

static struct keybind_t keybinds[N_KEYBIND];

static int add_history(struct shell *shell, char *line) {
    if (shell->history_len >= SHELL_HISTORY_SIZE) {
        shell->history_len = 0;
    }
    strcpy(shell->history_buf[shell->history_len++], line);
    return 0;
}

static void load_history(struct shell *shell) {
    FILE *fp = fopen(shell->history_file, "r");
    if (!fp) {
        perror("fopen");
        return;
    }

    char line[1024];
    // 逐行读取, 并去掉结尾的换行
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        add_history(shell, line);
    }
    fclose(fp);
}

static void save_history(struct shell *shell) {
    FILE *fp = fopen(shell->history_file, "w");
    if (!fp) {
        perror("fopen");
        printf("%s\n", shell->history_file);
        return;
    }
    for (int i = 0; i < shell->history_len; i++) {
        fputs(shell->history_buf[i], fp);
        fputc('\n', fp);
    }
    fclose(fp);
}

static void try_autocomplete(struct shell *shell) {
    // only try autocomplete when cursor is at the end
    if (shell->cursor_idx != shell->cmd_idx || shell->cmd_idx == 0)
        return;

    for (int i = shell->history_len - 1; i >= 0; i--) {
        if (strncmp(shell->history_buf[i], shell->cmd_buf, shell->cmd_idx) == 0) {
            shell->history_p = shell->history_buf[i] + shell->cmd_idx;
            // 如果是全部匹配直接返回
            if (shell->cmd_idx == (int)strlen(shell->history_buf[i])) {
                return;
            }
            printf_grey("%s", shell->history_p);
            CURSOR_LEFT((int)strlen(shell->history_p));
            return;
        }
    }
}

static KeyBind handle_control_serial(struct keybind_t *keybind, void *data) {
    char c;
    struct shell *shell = data;
    read(STDIN_FILENO, &c, 1);
    ASSERT(c == '[');
    read(STDIN_FILENO, &c, 1);
    for (int i = 0; i < N_KEYBIND; i++) {
        if (c == (char)shell->keybinds[i].key) {
            return shell->keybinds[i].func(&shell->keybinds[i], data);
        }
    }
    ASSERT(0);
}

static KeyBind handle_cursor_left(struct keybind_t *keybind, void *data) {
    // DEBUG("called cursor_left\n");
    struct shell *shell = data;
    if (shell->cursor_idx > 0) {
        shell->cursor_idx--;
        CURSOR_LEFT(1);
    }
    return keybind->key;
}

static KeyBind handle_cursor_right(struct keybind_t *keybind, void *data) {
    // DEBUG("called cursor_right\n");
    struct shell *shell = data;
    if (shell->cursor_idx < shell->cmd_idx) {
        shell->cursor_idx++;
        CURSOR_RIGHT(1);
    }
    return keybind->key;
}

static KeyBind handle_cursor_up(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    shell->history_idx = (shell->history_idx - 1 + shell->history_len) % shell->history_len;
    char *line = shell->history_buf[shell->history_idx];
    // copy line to cmd_buf
    strcpy(shell->cmd_buf, line);
    shell->cmd_idx = strlen(line);
    shell->cursor_idx = shell->cmd_idx;

    CLEAR_LINE();
    printf("%s%s", shell->prompt, line);
    fflush(stdout);
    return keybind->key;
}

static KeyBind handle_cursor_down(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    shell->history_idx = (shell->history_idx + 1) % shell->history_len;
    char *line = shell->history_buf[shell->history_idx];
    // copy line to cmd_buf
    strcpy(shell->cmd_buf, line);
    shell->cmd_idx = strlen(line);
    shell->cursor_idx = shell->cmd_idx;

    CLEAR_LINE();
    printf("%s%s", shell->prompt, line);
    fflush(stdout);
    return keybind->key;
}

static KeyBind handle_cursor_home(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    if (shell->cmd_idx == 0)
        return keybind->key;

    CURSOR_LEFT(shell->cmd_idx);
    shell->cursor_idx = 0;
    return keybind->key;
}

static KeyBind handle_cursor_end(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    if (shell->cursor_idx == shell->cmd_idx)
        return keybind->key;
    CURSOR_RIGHT(shell->cmd_idx - shell->cursor_idx);
    shell->cursor_idx = shell->cmd_idx;
    return keybind->key;
}

static KeyBind handle_exit_shell(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    shell->args.argc = 0;
    printf("\n");
    return keybind->key;
}

static KeyBind handle_clear_cmdline_before_cursor(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    // 将当前光标位置及之后的字符前移到开头
    for (int i = 0; i < shell->cmd_idx - shell->cursor_idx; i++) {
        shell->cmd_buf[i] = shell->cmd_buf[i + shell->cursor_idx];
    }
    CURSOR_LEFT(shell->cursor_idx);
    write(STDIN_FILENO, shell->cmd_buf, shell->cmd_idx - shell->cursor_idx);
    for (int i = 0; i < shell->cursor_idx; i++) {
        printf(" ");
    }
    CURSOR_LEFT(shell->cmd_idx);
    shell->cmd_idx -= shell->cursor_idx;
    shell->cursor_idx = 0;

    return keybind->key;
}

static KeyBind handle_clear_cmdline_after_cursor(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    if (shell->cursor_idx == shell->cmd_idx)
        return keybind->key;

    for (int i = shell->cursor_idx; i < shell->cmd_idx; i++) {
        printf(" ");
    }
    CURSOR_LEFT(shell->cmd_idx - shell->cursor_idx);
    shell->cmd_idx = shell->cursor_idx;

    return keybind->key;
}

// static void eval_cmdline(void *data) {
//     cmd_buf[cmd_idx] = '\0';
//     DEBUG("cmd_buf: %s\n", cmd_buf);
//     cmd_idx = 0;
//     cursor_idx = 0;
//     printf("\n");
//     eval();
//     printf("\n%s", prompt);
//     fflush(stdout);
//     update_tty_keybind();

// }

static KeyBind handle_clear_screen(struct keybind_t *keybind, void *data) {
    CLEAR_SCREEN();
    CURSOR_RESET();
    struct shell *shell = data;
    printf("%s", shell->prompt);
    fflush(stdout);
    if (shell->cmd_idx > 0) {
        write(STDIN_FILENO, shell->cmd_buf, shell->cmd_idx);
        if (shell->cursor_idx != shell->cmd_idx) {
            CURSOR_LEFT(shell->cmd_idx - shell->cursor_idx);
        }
    }

    return keybind->key;
}

static KeyBind handle_tabcomplete(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    if (shell->history_p && shell->cmd_idx == shell->cursor_idx) {
        strncpy(shell->cmd_buf + shell->cmd_idx, shell->history_p, (int)strlen(shell->history_p));
        shell->cmd_idx += (int)strlen(shell->history_p);
        shell->cursor_idx = shell->cmd_idx;
        printf("%s", shell->history_p);
        fflush(stdout);
    }
    return keybind->key;
}

static KeyBind handle_enter(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    shell->cmd_buf[shell->cmd_idx] = '\0';
    parseline(shell->cmd_buf, &shell->args);
    add_history(shell, shell->cmd_buf);
    shell->history_idx = shell->history_len;  // reset history index

    struct shell_event *event = &shell->event;
    event->argc = shell->args.argc;
    event->argv = shell->args.argv;
    event->keybind = keybind;

    shell->cmd_idx = 0;
    shell->cursor_idx = 0;
    printf("\n");
    fflush(stdout);

    return keybind->key;
}

static KeyBind handle_backspace(struct keybind_t *keybind, void *data) {
    struct shell *shell = data;
    if (shell->cursor_idx == 0)
        return keybind->key;
    if (shell->cursor_idx == shell->cmd_idx) {
        shell->cursor_idx--;
        shell->cmd_idx--;
        DELETE_CHAR();
    } else {
        // 将当前光标位置及之后的字符前移一位
        for (int i = shell->cursor_idx; i < shell->cmd_idx; i++) {
            shell->cmd_buf[i - 1] = shell->cmd_buf[i];
        }
        shell->cursor_idx--;
        shell->cmd_idx--;
        DELETE_CHAR();
        write(STDIN_FILENO, shell->cmd_buf + shell->cursor_idx, shell->cmd_idx - shell->cursor_idx);
        printf(" \b");
        CURSOR_LEFT(shell->cmd_idx - shell->cursor_idx);
    }
    try_autocomplete(shell);
    return keybind->key;
}

int shell_run(struct shell *shell) {
    char c;

    printf("%s", shell->prompt);
    fflush(stdout);
    while (1) {
        read(STDIN_FILENO, &c, 1);

        if (shell->history_p && shell->cmd_idx == shell->cursor_idx) {
            CLEAR_CHAR_AFTER((int)strlen(shell->history_p));
        }

        int flag = 0;
        for (int i = 0; i < shell->keybinds_len; i++) {
            if (c == (char)shell->keybinds[i].key) {
                KeyBind key = shell->keybinds[i].func(&keybinds[i], shell);
                // return 1 if should call user to handle next or exit shell
                if (key == KEY_ENTER || key == KEY_ESC || key == shell->keybinds[K_EOF].key) {
                    return 1;
                }
                flag = 1;
                break;
            }
        }

        if (flag)
            continue;

        if (shell->cursor_idx == shell->cmd_idx) {
            shell->cmd_buf[shell->cmd_idx++] = c;
            shell->cursor_idx++;
            write(STDOUT_FILENO, &c, 1);  // 显示输入字符
        } else {
            // 如果光标在开头或中间
            // 将当前光标位置及之后的字符后移一位
            for (int i = shell->cmd_idx; i > shell->cursor_idx; i--) {
                shell->cmd_buf[i] = shell->cmd_buf[i - 1];
            }

            // 插入新字符
            shell->cmd_buf[shell->cursor_idx++] = c;
            shell->cmd_idx++;
            write(STDOUT_FILENO, shell->cmd_buf + shell->cursor_idx - 1, shell->cmd_idx - shell->cursor_idx + 1);
            CURSOR_LEFT(shell->cmd_idx - shell->cursor_idx);
        }
        try_autocomplete(shell);
    }
}

static void init_keybind(struct shell *shell) {
    keybinds[K_CTL_SERIAL].key = '\033';
    keybinds[K_CTL_SERIAL].func = handle_control_serial;

    keybinds[K_UP].key = KEY_UP;
    keybinds[K_UP].func = handle_cursor_up;

    keybinds[K_DOWN].key = KEY_DOWN;
    keybinds[K_DOWN].func = handle_cursor_down;

    keybinds[K_LEFT].key = KEY_LEFT;
    keybinds[K_LEFT].func = handle_cursor_left;
    keybinds[K_RIGHT].key = KEY_RIGHT;
    keybinds[K_RIGHT].func = handle_cursor_right;

    keybinds[K_TAB].key = KEY_TAB;
    keybinds[K_TAB].func = handle_tabcomplete;

    keybinds[K_ENTER].key = KEY_ENTER;
    keybinds[K_ENTER].func = handle_enter;

    keybinds[K_HOME].key = KEY_HOME;
    keybinds[K_HOME].func = handle_cursor_home;

    keybinds[K_END].key = KEY_END;
    keybinds[K_END].func = handle_cursor_end;

    struct termios current_termios;
    tcgetattr(STDIN_FILENO, &current_termios);
    keybinds[K_EOF].key = current_termios.c_cc[VEOF];
    keybinds[K_EOF].func = handle_exit_shell;

    keybinds[K_ERASE].key = current_termios.c_cc[VERASE];
    keybinds[K_ERASE].func = handle_backspace;

    keybinds[K_KILL].key = current_termios.c_cc[VKILL];
    keybinds[K_KILL].func = handle_clear_cmdline_before_cursor;

    keybinds[K_CLEAR_SCREEN].key = KEY_CLEAR_SCREEN;
    keybinds[K_CLEAR_SCREEN].func = handle_clear_screen;

    keybinds[K_CLEAR_AFTER_CURSOR].key = KEY_CLEAR_AFTER;
    keybinds[K_CLEAR_AFTER_CURSOR].func = handle_clear_cmdline_after_cursor;

    shell->keybinds = keybinds;
    shell->keybinds_len = N_KEYBIND;
}

struct shell *create_shell(char *prompt) {
    struct shell *shell = malloc(sizeof(struct shell));
    memset(shell, 0, sizeof(struct shell));
    shell->prompt = prompt;

    shell->init_signal = NULL;
    shell->init_keybind = init_keybind;
    shell->enable_history_search_complete = true;
    return shell;
}

int init_shell(struct shell *shell) {
    if (shell->init_signal) {
        shell->init_signal();
    }
    if (shell->init_keybind) {
        shell->init_keybind(shell);
    }
    if (shell->history_file) {
        if (path_exist(shell->history_file)) {
            load_history(shell);
        }
    }
    enable_raw_mode();

    return 0;
}

void free_shell(struct shell *shell) {
    if (!shell) {
        return;
    }

    free(shell);
    if (shell->args.argv) {
        free_cmd_arg(&shell->args);
    }

    if (shell->history_file) {
        save_history(shell);
    }

    disable_raw_mode();
}

/**
 * @brief 解析命令行
 *
 * @param cmdbuf
 * @param args
 * @return int
 */
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