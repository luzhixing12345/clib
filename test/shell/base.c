
#include <clib/clib.h>

int main(int argc, char **argv) {
    struct shell *shell = create_shell("> ");
    shell->history_file = "shell/shell_history";
    init_shell(shell);

    while (shell_run(shell)) {
        struct shell_event *event = &shell->event;
        if (event->argc > 0 && !strcmp(event->argv[0], "exit")) {
            break;
        }
        for (int i = 0; i < event->argc; i++) {
            printf("%s ", event->argv[i]);
        }
        printf("\n");
    }

    free_shell(shell);
    return 0;
}
