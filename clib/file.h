
#pragma once

#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

#define RWX (S_IRWXU | S_IRWXG | S_IRWXO)
#define RW  (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/**
 * @brief delete dir recursively
 *
 * @param path
 * @return int
 */
int del_dir(const char *path);

/**
 * @brief if path exist
 *
 * @param path
 * @return int
 */
int path_exist(const char *path);

bool is_mounted(struct stat *st);