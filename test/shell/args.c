
#include <clib/clib.h>

#include <stdio.h>

int main(int argc, char **argv) {
    struct cmd_arg args;

    if (parseline("ls -l", &args) != 0) {
        return 1;
    }
    
    printf("argc = %d\n", args.argc);
    for (int i = 0; i < args.argc; i++) {
        printf("%s\n", args.argv[i]);
    }

    free_cmd_arg(&args);

    const char *cmd = "./kemu --kernel ~/klinux/arch/x86_64/boot/bzImage --disk ~/workspace/disk/ubuntu.raw --append \"root=/dev/vda2 quiet\"";
    struct cmd_arg args2;
    if (parseline(cmd, &args2) != 0) {
        return 1;
    }
    
    printf("argc = %d\n", args2.argc);
    for (int i = 0; i < args2.argc; i++) {
        printf("%s\n", args2.argv[i]);
    }

    free_cmd_arg(&args);
    return 0;
}
