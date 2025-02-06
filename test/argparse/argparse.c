#include <clib/clib.h>

int main(int argc, const char **argv) {
    int integer;
    char *str = NULL, *dest;
    int src;
    int *other_numbers;
    argparse_option options[] = {
        ARG_BOOLEAN(NULL, "-h", "--help", "show help information", NULL, "help"),
        ARG_BOOLEAN(NULL, "-v", "--version", "show version", NULL, "version"),
        ARG_INT(&integer, "-i", "--input", "input file", " <NUMBER>", "input"),
        ARG_STR(&str, "-s", "--string", NULL, " <STRING>", NULL),
        ARG_STR_GROUP(&dest, NULL, NULL, "destination", NULL, "dest"),
        ARG_INT_GROUP(&src, NULL, NULL, "source", NULL, "src"),
        ARG_INTS_GROUP(&other_numbers, NULL, NULL, "catch the other number ...", NULL, "other-number"),
        ARG_END()};

    argparse parser;
    argparse_init(&parser, options, 0);
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
    if (arg_ismatch(&parser, "input")) {
        printf("i = %d\n", integer);
    }
    if (str) {
        printf("s = %s\n", str);
    }
    if (arg_ismatch(&parser, "dest")) {
        printf("dest = %s\n", dest);
    }
    if (arg_ismatch(&parser, "src")) {
        printf("src = %d\n", src);
    }
    int n = arg_ismatch(&parser, "other-number");
    for (int i = 0; i < n; i++) {
        printf("other number[%d] = %d\n", i, other_numbers[i]);
    }
    free_argparse(&parser);
    return 0;
}