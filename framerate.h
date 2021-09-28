#ifndef _FRAMERATE_H_
#define _FRAMERATE_H_

#include "typedefs.h"
#include "platform.h"

#include <GLFW/glfw3.h>
#include <Windows.h>

struct FramerateData {
    u32 prev_time;
    u32 current_time;
    i32 time_slept;
    u32 fps;

    FramerateData(u32 fps) {
        prev_time = glfwGetTime() * 1000;
        current_time = prev_time;
        time_slept = 0;
        this->fps = fps;
    }
};

void wait_until_next_frame(FramerateData *fr) {
    fr->current_time = glfwGetTime() * 1000;
    u32 time_taken = fr->current_time - fr->prev_time - fr->time_slept;
    fr->prev_time = fr->current_time;

    fr->time_slept = 1000 / fr->fps - time_taken;
    if (fr->time_slept < 0) {
        fr->time_slept = 0;
    }

    platform_sleep(fr->time_slept);
}

#endif
