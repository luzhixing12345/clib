
#include <clib/clib.h>
#include <unistd.h>  // For usleep

void my_end_format(struct tqdm *progress, int x) {
    fprintf(stdout,
            " my_end_format %d", x);
}

int main() {
    int iter;
    struct tqdm progress;

    iter = 300;
    init_tqdm(&progress, iter);
    for (int i = 0; i < iter; i++) {
        update_tqdm(&progress, i);
        usleep(10000);
    }

    init_tqdm(&progress, iter);
    progress.bar_len = 50;
    progress.filled_char = "*";
    progress.empty_char = "-";
    progress.end_format = my_end_format;
    for (int i = 0; i < iter; i++) {
        update_tqdm(&progress, i);
        usleep(10000);
    }
    return (0);
}