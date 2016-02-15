/*
 * Copyright (C) 2016 Ingemar Ådahl
 *
 * This file is part of remote-input.
 *
 * remote-input is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * remote-input is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with remote-input.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <linux/limits.h>


char const * const PRIORITY_NAMES[] = {
    "", /* EMERG - unused*/
    "FATAL",
    " CRIT",
    "ERROR",
    " WARN",
    "NOTIC",
    " INFO",
    "DEBUG"
};


PRINTF_TYPE(2,3)
void stdio_log(int priority, const char* format, ...) {
    va_list args;
    va_start(args, format);

    FILE* stream = priority < LOG_WARNING ? stderr : stdout;
    fflush(stdout);
    fflush(stderr);

    fprintf(stream, "[%s] ", PRIORITY_NAMES[priority]);
    vfprintf(stream, format, args);
    fprintf(stream, "\n");

    va_end(args);
}

PRINTF_TYPE(2,3)
void(*__log_function)(int priority, const char* format, ...) = &stdio_log;

enum log_target selected_target = STDIO;

void log_set_target(enum log_target target) {
    if (selected_target == target) {
        return;
    }

    selected_target = target;

    if (selected_target == STDIO) {
        closelog();
        __log_function = &stdio_log;
        return;
    }

    if (selected_target == SYSLOG) {
        openlog(NULL, LOG_NDELAY | LOG_PID, LOG_USER);
        __log_function = &syslog;
    }
}

void __log_errno(const char* file, int line, const char* format, ...) {
    char msg_buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(msg_buf, sizeof(msg_buf), format, args);
    va_end(args);

    if (file != NULL && line >= 0) {
        LOG(ERROR, "%s:%d: %s: %s", file, line, msg_buf, strerror(errno));
        return;
    }

    LOG(ERROR, "%s: %s", msg_buf, strerror(errno));
}
