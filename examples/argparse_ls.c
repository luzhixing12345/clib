#include <clib/clib.h>
#include <string.h>
#include "clib/argparse.h"

int main(int argc, const char **argv) {
    char *cmd;
    argparse_option options[] = {
        ARG_BOOLEAN(NULL, "-h", "--help", "show help information", NULL, "help"),
        ARG_BOOLEAN(NULL, "-v", "--version", "show version information", NULL, "version"),
        ARG_STR(&cmd, NULL, "--", NULL, NULL, "cmd"),
        ARG_END()};

    argparse parser;
    argparse_init(&parser, options, ARGPARSE_ENABLE_CMD);
    argparse_describe(&parser,
                      "main",
                      "\nA brief description of what the program does and how it works.",
                      "\nAdditional description of the program after the description of the arguments.");
    argparse_parse(&parser, argc, argv);
    if (arg_ismatch(&parser, "help")) {
        argparse_info(&parser);
    }
    if (arg_ismatch(&parser, "version")) {
        printf("v0.0.1\n");
    }
    if (arg_ismatch(&parser, "cmd")) {
        printf("cmd: %s\n", cmd);
    }
    
    free_argparse(&parser);
    return 0;
}