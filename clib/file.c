
#include "file.h"

#include <dirent.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief delete dir recursively
 *
 * @param path
 * @return int
 */
int del_dir(const char *path) {
    DIR *dir = opendir(path);
    struct dirent *entry;
    char filepath[1024];

    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // 忽略 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构造文件或目录的完整路径
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(filepath, &statbuf) == -1) {
            perror("stat");
            closedir(dir);
            return -1;
        }

        // 如果是目录,递归删除
        if (S_ISDIR(statbuf.st_mode)) {
            if (del_dir(filepath) == -1) {
                closedir(dir);
                return -1;
            }
        } else {  // 如果是文件,删除文件
            if (remove(filepath) == -1) {
                perror("remove");
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);

    // 删除当前目录
    if (rmdir(path) == -1) {
        perror("rmdir");
        return -1;
    }

    return 0;
}

int path_exist(const char *path) {
    return access(path, F_OK) == 0;
}


bool is_mounted(struct stat *st) {
    struct stat st_buf;
    struct mntent *mnt;
    FILE *f;

    f = setmntent("/proc/mounts", "r");
    if (!f)
        return false;

    while ((mnt = getmntent(f)) != NULL) {
        if (stat(mnt->mnt_fsname, &st_buf) == 0 && S_ISBLK(st_buf.st_mode) && st->st_rdev == st_buf.st_rdev) {
            fclose(f);
            return true;
        }
    }

    fclose(f);
    return false;
}


#include <linux/limits.h>
#include <stdio.h>
#include <sys/stat.h>

/**
 * @brief 打开一个目录并读取该目录下所有的文件和目录
 *
 * @param path 路径名
 * @param flag
    - DIR_IGNORE_HIDDEN: 不包含.开头的
    - DIR_IGNORE_CURRENT: 不包含.和..
    - DIR_ALL: 全部包含
 * @return Dir* 需要调用 freedir 释放
 */
Dir* read_dir(const char* path, int flag) {
    DIR* dir;
    struct dirent* entry;

    int length;
    dir = opendir(path);
    if (dir == NULL) {
        // 12 是 "open failed " 的长度
        static char error_info[PATH_MAX + 12];
        memset(error_info, 0, PATH_MAX + 12);
        sprintf(error_info, "open failed %s", path);
        perror(error_info);
        exit(1);
    }

    Dir* directory = (Dir*)malloc(sizeof(Dir));
    memset(directory, 0, sizeof(Dir));
    while ((entry = readdir(dir)) != NULL) {
        if ((flag & DIR_IGNORE_HIDDEN) && entry->d_name[0] == '.') {
            continue;
        }
        if ((flag & DIR_IGNORE_CURRENT) && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))) {
            continue;
        }
        directory->count++;
        if (entry->d_type == DT_DIR) {
            // 目录
            directory->d_count++;
        } else {
            directory->f_count++;
        }
    }
    rewinddir(dir);
    directory->dp = (File**)malloc(sizeof(File*) * directory->count);
    for (int i = 0; i < directory->count; i++) {
        directory->dp[i] = (File*)malloc(sizeof(File));
    }
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if ((flag & DIR_IGNORE_HIDDEN) && entry->d_name[0] == '.') {
            continue;
        }
        if ((flag & DIR_IGNORE_CURRENT) && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))) {
            continue;
        }
        length = strlen(entry->d_name);
        strncpy(directory->dp[i]->name, entry->d_name, length);
        directory->dp[i]->name[length] = 0;
        directory->dp[i]->type = entry->d_type;
        i++;
    }
    length = strlen(path);
    strncpy(directory->name, path, length);
    closedir(dir);
    return directory;
}

/**
 * @brief 释放目录分配的内存
 *
 * @param dir
 */
void freedir(Dir* directory) {
    directory->parent = NULL;
    for (int i = 0; i < directory->count; i++) {
        free(directory->dp[i]);
    }
    free(directory->dp);
    free(directory);
}

/**
 * @brief 连接路径, 可变参数, 最后一个参数传 NULL
 *
 * @param path
 * @param ...
 * @return char*
 */
char* path_join(const char* path, ...) {
    static char result[PATH_MAX];
    memset(result, 0, PATH_MAX);
    va_list args;
    char* arg;
    size_t len = strlen(path);
    strncpy(result, path, len);
    va_start(args, path);

    while ((arg = va_arg(args, char*))) {
        size_t arg_len = strlen(arg);
        if (len + arg_len + 1 > PATH_MAX) {
            fprintf(stderr, "Error: maximum path length exceeded\n");
            return NULL;
        }
        strcat(result, "/");
        len += arg_len + 1;
        strncat(result, arg, arg_len);
    }
    va_end(args);
    return result;
}

/**
 * @brief 获取路径的最后一个目录
 *
 * @param path
 * @return const char*
 */
const char* get_last_path(const char* path) {
    static char result[PATH_MAX];
    const char* p = strrchr(path, '/');
    if (p == NULL) {
        // 如果路径中没有斜杠,则返回整个路径
        return path;
    } else {
        // 如果路径中有斜杠,则返回最后一个斜杠后面的部分
        const char* p2 = strrchr(path, '/') + 1;
        if (*p2 == '\0') {
            // 如果最后一个斜杠后面没有内容,则返回前一个斜杠后面的部分
            const char* p3 = strrchr(path, '/') - 1;
            int len = p3 - path + 1;
            strncpy(result, path, len);
            result[len] = '\0';
            return result;
        } else {
            // 如果最后一个斜杠后面有内容,则返回最后一个斜杠后面的部分
            return p2;
        }
    }
}

/**
 * @brief 返回文件的[读/写/执行]权限的字符串
 *
 * @param mode
 * @return char*
 */
char* stat_access_mode(mode_t mode) {
    static char buf[11];
    memset(buf, 0, 11);
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    buf[10] = '\0';
    if (S_ISREG(mode)) {
        buf[0] = '-';
        if (mode & S_ISUID) {
            buf[3] = 'S';
        } else if (mode & S_ISGID) {
            buf[6] = 'S';
        }
    } else if (S_ISDIR(mode)) {
        buf[0] = 'd';
        if (mode & S_ISVTX) {
            buf[9] = 't';
        }
    } else if (S_ISCHR(mode)) {
        buf[0] = 'c';
    } else if (S_ISBLK(mode)) {
        buf[0] = 'b';
    } else if (S_ISFIFO(mode)) {
        buf[0] = 'p';
    } else if (S_ISLNK(mode)) {
        return (char*)"lrwxrwxrwx";
    } else if (S_ISSOCK(mode)) {
        buf[0] = 's';
    } else {
        buf[0] = '-';
        // UNKNOWN type?
    }
    // mode_t umask_val = umask(0);                // 获取当前 umask 值
    // umask(umask_val);                           // 恢复原来的 umask 值
    // mode_t effective_mode = mode & ~umask_val;  // 计算生效的权限值

    return buf;
}