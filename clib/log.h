#ifndef LOGGING_H
#define LOGGING_H

#include <assert.h>
#include <stdlib.h>

#define LOG_ERR     0
#define LOG_WARNING 1
#define LOG_INFO    2
#define LOG_DEBUG   3

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#define EMERG(...)   __LOG(LOG_EMERG, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ALERT(...)   __LOG(LOG_ALERT, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define CRIT(...)    __LOG(LOG_CRIT, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ERR(...)     __LOG(LOG_ERR, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define WARNING(...) __LOG(LOG_WARNING, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define NOTICE(...)  __LOG(LOG_NOTICE, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define INFO(...)    __LOG(LOG_INFO, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG(...)   __LOG(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DIE(...)                                                         \
    {                                                                    \
        __LOG(LOG_ERR, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        exit(1);                                                         \
    }

#define DEFAULT_LOG_FILE  NULL

#define DEFAULT_LOG_LEVEL LOG_DEBUG
#define ASSERT(assertion)                        \
    ({                                           \
        if (!(assertion)) {                      \
            WARNING("ASSERT FAIL: " #assertion); \
            assert(assertion);                   \
        }                                        \
    })

void __LOG(int level, const char *file, const char *func, int line, const char *format, ...);
void log_set_level(int new_level);
int log_init(const char *path);

#endif
