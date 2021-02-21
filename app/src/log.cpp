#include <cstdio>
#include <cstdarg>
#include "log.h"
#include <mutex>

std::mutex mu;

void debugf(const char *fmt...) {
    std::unique_lock<std::mutex> lock(mu);
    printf("DEBUG ");
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);
    printf("\n");
    fflush(stdout);
}

void infof(const char *fmt...) {
    std::unique_lock<std::mutex> lock(mu);
    printf("INFO  ");
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);
    printf("\n");
    fflush(stdout);
}

void errorf(const char *fmt...) {
    std::unique_lock<std::mutex> lock(mu);
    printf("ERROR ");
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);
    printf("\n");
    fflush(stdout);
}

void warnf(const char *fmt, ...) {
    std::unique_lock<std::mutex> lock(mu);
    printf("WARN  ");
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);
    printf("\n");
    fflush(stdout);
}
