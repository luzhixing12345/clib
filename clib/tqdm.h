#pragma once
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

// Structure to hold progress bar information
struct tqdm {
    int total;
    int count;
    int bar_len;
    double average_time;
    double last_time;
    double total_time;

    // could be configured
    char *filled_char;  // "█"
    char *empty_char;   // " "
    void (*end_format)(struct tqdm *tqdm, int x);
};

int init_tqdm(struct tqdm *progress, int total);
void update_tqdm(struct tqdm *progress, int x);
