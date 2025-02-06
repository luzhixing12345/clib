
#pragma once

#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

#define RWX   (S_IRWXU | S_IRWXG | S_IRWXO)                                                              // 0777
#define D_RWX (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)  // 0755
#define RW    (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)                                // 0666

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