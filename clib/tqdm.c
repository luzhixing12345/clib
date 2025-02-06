#include "tqdm.h"

#include <math.h>

// https://github.com/soulee-dev/cqdm

#define BASE_BAR_SIZE 30

// Returns the current time in milliseconds
long long get_msec_now() {
    struct timeval time;

    if (gettimeofday(&time, NULL) == -1)
        return (0);
    return (long long)time.tv_sec * 1000LL + (long long)time.tv_usec / 1000LL;
}

static int num_width(int num) {
    int width = 1;
    while (num >= 10) {
        num /= 10;
        width++;
    }
    return width;
}

static void tqdm_end_format(struct tqdm *progress, int x) {
    fprintf(stdout,
            " %*d / %d [%.2fs<%.2fs, %.2fit/s]",
            num_width(progress->total),
            x + 1,
            progress->total,
            progress->total_time,
            progress->average_time * progress->total,
            1 / progress->average_time);
}

int init_tqdm(struct tqdm *progress, int total) {
    progress->total = total;
    progress->count = 0;
    progress->average_time = 0;
    progress->last_time = get_msec_now();
    progress->total_time = 0;
    
    // could be configured
    progress->bar_len = BASE_BAR_SIZE;
    progress->filled_char = "█";
    progress->empty_char = " ";
    progress->end_format = tqdm_end_format;
    return 0;
}

void update_tqdm(struct tqdm *progress, int x) {
    long long now;
    double temp;

    now = get_msec_now();

    progress->count++;
    progress->total_time += ((now - progress->last_time) / 1000);
    progress->average_time = progress->total_time / progress->count;
    progress->last_time = now;

    temp = ((double)(x + 1) / (double)progress->total);

    fprintf(stdout, "\r%3d%% | ", (int)(temp * 100));

    int filled_bars = (int)(temp * progress->bar_len);
    int empty_bars = progress->bar_len - filled_bars;
    for (int i = 0; i < filled_bars; i++) fprintf(stdout, "%s", progress->filled_char);
    for (int i = 0; i < empty_bars; i++) fprintf(stdout, "%s", progress->empty_char);
    fprintf(stdout, " |");

    progress->end_format(progress, x);

    if (temp == 1) {
        fprintf(stdout, "\n");
    }
    fflush(stdout);
}