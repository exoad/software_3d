#ifndef LOGGER_H
#define LOGGER_H

#include "shared.h"

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

#ifndef LOGGER_DISABLE_GLOBAL

Void log_msg(LogLevel level, CharSeq file, Int32 line, CharSeq fmt, ...);

#define LOG_D(...) log_msg(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_I(...) log_msg(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_W(...) log_msg(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_E(...) log_msg(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#else

#define LOG_D(fmt, ...) ((Void) 0)
#define LOG_I(fmt, ...) ((Void) 0)
#define LOG_W(fmt, ...) ((Void) 0)
#define LOG_E(fmt, ...) ((Void) 0)

#endif

#endif // LOGGER_H
