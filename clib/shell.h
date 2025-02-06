
#pragma once

struct cmd_arg {
    int argc;
    char **argv;

    // private
    char *_cmdbuf;
};

int parseline(const char *cmdbuf, struct cmd_arg *args);
void free_cmd_arg(struct cmd_arg *args);