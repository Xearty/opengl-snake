#define CELL_COUNT 15
#define GAP 12.0f
#define ARR_SIZE(arr) (sizeof(arr) / sizeof(*arr))

#include "typedefs.h"
#include "platform.h"
#include "util.h"
#include "cell.h"
#include "bridge.h"
#include "grid.h"
#include "framerate.h"
#include "snake.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <time.h>

void key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods) {
    if (action == GLFW_PRESS) {
        GameState *game = (GameState *)glfwGetWindowUserPointer(window);

        #define KEY_ACTION(BUTTON, ACTION) case GLFW_KEY_##BUTTON: ACTION; break
        switch (key) {
            KEY_ACTION(P, game->paused = !game->paused);
            KEY_ACTION(W, game->snake.should_grow = true);
            KEY_ACTION(R, restart_game(game));
            KEY_ACTION(ESCAPE, glfwSetWindowShouldClose(window, GL_TRUE));

            case GLFW_KEY_UP:
            case GLFW_KEY_RIGHT:
            case GLFW_KEY_DOWN:
            case GLFW_KEY_LEFT: {
                if (!game->paused) push_queue(&game->turns_queue, key);
            } break;
        }
    }
}

i32 main() {
    srand(time(0));
    glfwInit();

    const bool32 is_fullscreen = true;
    glm::ivec2 window_size = { 800, 800 };
    GLFWmonitor *monitor = NULL;

    i32 dim_diff = 0;
    if (is_fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        window_size = { mode->width, mode->height };
        dim_diff = window_size.x - window_size.y;
    }

    GLFWwindow *window = glfwCreateWindow(window_size.x, window_size.y, "OpenGL Snake", monitor, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(dim_diff / 2, 0, window_size.y, window_size.y);
    glfwSetKeyCallback(window, key_callback);

    ObjectData cell = configure_cell(window_size);
    ObjectData bridge = configure_bridge(window_size);
    ObjectData grid = configure_grid(window_size);

    FramerateData framerate = {10};
    GameState game = {};
    restart_game(&game);
    glfwSetWindowUserPointer(window, &game);

    float cell_height = (float)window_size.y / CELL_COUNT;
    glm::vec2 cell_size = glm::vec2(cell_height, cell_height);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (!game.is_over && !game.paused) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            update_snake(&game);

            render_food(&cell, game.food_pos);
            render_snake(&game.snake.tail, &cell, &bridge, cell_size);
            render_object(&grid);

            glfwSwapBuffers(window);
        }

        wait_until_next_frame(&framerate);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

