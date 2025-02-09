
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdarg.h>

#ifdef USE_DEBUG
    #define Debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define Debug(fmt, ...) 
#endif

#endif