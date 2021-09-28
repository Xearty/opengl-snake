#define CELL_COUNT 15
#define ARR_SIZE(arr) (sizeof(arr) / sizeof(*arr))
#define RGB_VAL(r, g, b) (float)(r)/255, (float)(g)/255, (float)(b)/255

#include "typedefs.h"
#include "platform.h"
#include "util.h"
#include "quad.h"
#include "grid.h"
#include "framerate.h"
#include "snake.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <time.h>
#include <string.h>

void key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    GameState *game = (GameState *)glfwGetWindowUserPointer(window);

    switch (key) {
        case GLFW_KEY_P: {
            game->paused = !game->paused;
        } break;

        case GLFW_KEY_ESCAPE: {
            glfwSetWindowShouldClose(window, GL_TRUE);
        } break;

        case GLFW_KEY_W: {
            game->snake.should_grow = true;
        } break;

        case GLFW_KEY_R: {
            restart_game(game);
        } break;

        case GLFW_KEY_UP:
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_DOWN:
        case GLFW_KEY_LEFT: {
            if (!game->paused) {
                push_queue(&game->turns_queue, key);
            }
        } break;
    }
}

i32 main() {
    srand(time(0));
    glfwInit();

    const bool32 is_fullscreen = true;
    glm::ivec2 window_size = glm::ivec2(800, 800);
    GLFWmonitor *monitor = NULL;

    if (is_fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        window_size = glm::ivec2(mode->width, mode->height);
    }
    
    GLFWwindow* window = glfwCreateWindow(window_size.x, window_size.y, "OpenGL Snake", monitor, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    if (is_fullscreen) {
        const i32 dim_diff = window_size.x - window_size.y;
        glViewport(dim_diff / 2, 0, window_size.y, window_size.y);
    } else {
        glViewport(0, 0, window_size.x, window_size.y);
    }

    glfwSetKeyCallback(window, key_callback);

    ObjectData grid = configure_grid(window_size);
    ObjectData quad = configure_quad(window_size);

    FramerateData framerate(15);
    GameState game = {};
    restart_game(&game);
    glfwSetWindowUserPointer(window, &game);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (game.paused) {
            platform_sleep(100);
            continue;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        update_snake(&game);

        render_food(&quad, game.food_pos);
        render_snake(&game.snake, &quad);
        render_object(&grid);

        glfwSwapBuffers(window);
        wait_until_next_frame(&framerate);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
