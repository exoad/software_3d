#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <string.h>

#ifndef LOGGER_DISABLE_GLOBAL

static CharSeq level_strings[] = {
    "DEBUG", "INFO", "WARN", "ERROR"
};

static CharSeq level_colors[] = {
    "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m"
};

Void log_msg(LogLevel level, CharSeq file, Int32 line, CharSeq fmt, ...)
{
    time_t t = time(null);
    struct tm* tm_info = localtime(&t);
    Int8 time_buf[26];
    strftime(time_buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    CharSeq filename = strrchr(file, '/');
    if(!filename)
    {
        filename = strrchr(file, '\\');
    }
    filename = filename ? filename + 1 : file;

    fprintf(stdout, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
            time_buf, level_colors[level], level_strings[level], filename, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);

    fprintf(stdout, "\n");
    fflush(stdout);
}

#endif
