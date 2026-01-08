/*
 * Copyright (c) 2010, Gerard Lledó Vives, gerard.lledo@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "log.h"

static int loglevel = DEFAULT_LOG_LEVEL;
static FILE *logfile = NULL;
static const char *loglevel_str[] = {
    [LOG_ERR] = "ERROR",
    [LOG_WARNING] = "WARN",
    [LOG_INFO] = "INFO",
    [LOG_DEBUG] = "DEBUG",
};

static const char *logcolor_str[] = {
    [LOG_ERR] = "\x1b[1;31m",
    [LOG_WARNING] = "\x1b[1;33m",
    [LOG_INFO] = "\x1b[1;32m",
    [LOG_DEBUG] = "\x1b[1;35m",  // purple
};

void __LOG(int level, const char *file, const char *func, int line, const char *format, ...) {
    if (level > LOG_LEVEL) {
        return;
    }
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    va_list ap;

    if (level < 0)
        return;
    if (level > loglevel)
        return;

    /* We have a lock here so different threads don interleave the log output */
    pthread_mutex_lock(&lock);
    va_start(ap, format);
    if (!logfile) {
        logfile = stdout;
    }
    if (isatty(fileno(logfile))) {
        fprintf(logfile,
                "[%s%-5s\x1b[0m][\x1b[90m%s:%d(%s)\x1b[0m] ",
                logcolor_str[level],
                loglevel_str[level],
                file,
                line,
                func);
    } else {
        fprintf(logfile, "[%s][%s:%d(%s)] ", loglevel_str[level], file, line, func);
    }
    vfprintf(logfile, format, ap);
    fprintf(logfile, "\n");
    fflush(logfile);
    va_end(ap);
    pthread_mutex_unlock(&lock);
}

void log_set_level(int new_level) {
    loglevel = new_level;
}

/**
 * @brief redirect log from stdout to a file
 *
 * @param path
 * @return int
 */
int log_init(const char *path) {
    if (path == NULL) {
        return 0;
    }

    if ((logfile = fopen(path, "w")) == NULL) {
        perror("open");
        return -1;
    }

    return 0;
}
