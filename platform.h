#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "typedefs.h"

#if defined(_WIN32)
    #include <Windows.h>
#elif defined(__unix__)
    #include <unistd.h>
#endif

void platform_sleep(u32 milliseconds) {
    #if defined(_WIN32)
        Sleep(milliseconds);
    #elif defined(__unix__)
        usleep(milliseconds * 1000);
    #endif
}

#endif
