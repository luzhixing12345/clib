
#pragma once
#include "keyboard.h"
#include <stdbool.h>
struct cmd_arg {
    int argc;
    char **argv;

    // private
    char *_cmdbuf;
};

struct shell_event {
    int argc;
    char **argv;
    struct keybind_t *keybind;
};

#define SHELL_HISTORY_SIZE 100
#define SHELL_CMD_BUF_SIZE 1024

struct shell {
    char *prompt;
    struct keybind_t *keybinds;
    int keybinds_len;
    void (*init_signal)();
    void (*init_keybind)(struct shell *shell);

    char cmd_buf[SHELL_CMD_BUF_SIZE];
    int cmd_idx;     // 参数长度
    int cursor_idx;  // 光标位置
    struct cmd_arg args;
    struct shell_event event;

    // history
    char *history_file;
    char history_buf[SHELL_HISTORY_SIZE][SHELL_CMD_BUF_SIZE];
    int history_len;
    int history_idx;

    // auto complete
    bool enable_history_search_complete;
    char *history_p;
};

struct shell *create_shell(char *prompt);
int init_shell(struct shell *shell);
int shell_run(struct shell *shell);
void free_shell(struct shell *shell);

int parseline(const char *cmdbuf, struct cmd_arg *args);
void free_cmd_arg(struct cmd_arg *args);