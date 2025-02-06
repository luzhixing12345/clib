#include <clib/clib.h>

char *output = NULL;
char **include_path = NULL;

char **files = NULL;
char *optimize = NULL;
char **warning = NULL;
char **library_path = NULL;
char **library_name = NULL;
char *c_standard = NULL;

int debug = 0;

int main(int argc, const char **argv) {
    argparse_option options[] = {
        ARG_BOOLEAN(NULL, "-c", NULL, "Compile and assemble, but do not link.", NULL, "compile"),
        ARG_STR(&output, "-o", NULL, "Place the output into <file>.", " <file>", NULL),
        ARG_STRS(&include_path, "-I", NULL, "Add <dir> to the end of the main include path.", "<dir>", "include"),
        ARG_STRS(
            &library_path, "-L", NULL, "Add <dir> to the end of the main library path.", "<dir>", "library_path"),
        ARG_STRS(&library_name, "-l", NULL, "Search <lib> in library path", "<lib>", "library_name"),
        ARG_BOOLEAN(&debug, "-g", NULL, "Generate debug information in default format", NULL, "debug"),
        ARG_STRS(&warning, "-W", NULL, "Warning information option", NULL, NULL),
        ARG_STR(&optimize, "-O", "--optimize", "optimization level to <number>.", "<number>", NULL),
        ARG_STR(&c_standard, NULL, "--std", "C Compile standard, supported: {c99}", "=<standard>", NULL),
        ARG_BOOLEAN(NULL, "-h", "--help", "show help information", NULL, "help"),
        ARG_BOOLEAN(NULL, "-v", "--version", "show version", NULL, "version"),
        ARG_STRS_GROUP(&files, NULL, NULL, NULL, NULL, "src"),
        ARG_END()};

    argparse parser;
    argparse_init(
        &parser, options, ARGPARSE_ENABLE_ARG_STICK | ARGPARSE_ENABLE_STICK | ARGPARSE_ENABLE_EQUAL);
    argparse_describe(&parser,
                           "kcc",
                           "Kamilu's C Compiler",
                           "document:   <https://github.com/luzhixing12345/kcc>\nbug report: "
                           "<https://github.com/luzhixing12345/kcc/issues>");
    argparse_parse(&parser, argc, argv);

    if (arg_ismatch(&parser, "help")) {
        argparse_info(&parser);
        free_argparse(&parser);
        return 0;
    }

    if (arg_ismatch(&parser, "version")) {
        printf("%s\n", VERSION);
        free_argparse(&parser);
        return 0;
    }

    if (arg_ismatch(&parser, "compile")) {
        printf("use -c\n");
    }
    if (debug) {
        printf("use -g\n");
    }

    int n = arg_ismatch(&parser, "src");
    for (int i = 0; i < n; i++) {
        printf("file[%d] = [%s]\n", i, files[i]);
    }

    n = arg_ismatch(&parser, "include");
    for (int i = 0; i < n; i++) {
        printf("include path[%d] = [%s]\n", i, include_path[i]);
    }

    n = arg_ismatch(&parser, "library_path");
    for (int i = 0; i < n; i++) {
        printf("library path[%d] = [%s]\n", i, library_path[i]);
    }

    n = arg_ismatch(&parser, "library_name");
    for (int i = 0; i < n; i++) {
        printf("library name[%d] = [%s]\n", i, library_name[i]);
    }

    if (c_standard) {
        printf("C standard = [%s]\n", c_standard);
    }

    if (optimize) {
        printf("optimize = [%s]\n", optimize);
    }

    if (output) {
        printf("output file = [%s]\n", output);
    }
    free_argparse(&parser);
    return 0;
}