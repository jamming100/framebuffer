#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <errno.h>
#include <string.h>

//#define DEBUG_LOG

#define Log(format,args...) fprintf(stdout,format,  ##args)
#define LogE(format, args...)                                   \
    fprintf(stderr, "File: "__FILE__", Line: %d: errno: %d, %s\n", __LINE__, errno, strerror(errno)); \
    fprintf(stderr, format, ##args)

#ifdef DEBUG_LOG
    #define LogD(format,args...) fprintf(stdout,format,  ##args)
    #define LogA(format,args...) fprintf(stdout,"File: "__FILE__", Line: %d:" format, __LINE__, ##args)
#else
#define LogD(format, args...)
#define LogA(format, args...)

#endif
#endif // LOG_H
