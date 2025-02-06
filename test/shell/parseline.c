
#include <clib/clib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    struct cmd_arg args;

    if (parseline("ls -l", &args) != 0) {
        return 1;
    }

    ASSERT(args.argc == 2);
    const char *expected[] = {"ls", "-l"};
    for (int i = 0; i < args.argc; i++) {
        ASSERT(strcmp(args.argv[i], expected[i]) == 0);
    }

    free_cmd_arg(&args);

    const char *cmd =
        "./kemu --kernel ~/klinux/arch/x86_64/boot/bzImage --disk ~/workspace/disk/ubuntu.raw --append "
        "\"root=/dev/vda2 quiet\"";
    struct cmd_arg args2;
    if (parseline(cmd, &args2) != 0) {
        return 1;
    }

    ASSERT(args2.argc == 7);
    const char *expected2[] = {"./kemu",
                               "--kernel",
                               "~/klinux/arch/x86_64/boot/bzImage",
                               "--disk",
                               "~/workspace/disk/ubuntu.raw",
                               "--append",
                               "root=/dev/vda2 quiet"};
    for (int i = 0; i < args2.argc; i++) {
        ASSERT(strcmp(args2.argv[i], expected2[i]) == 0);
    }

    free_cmd_arg(&args);
    return 0;
}
