#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "util.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <deque>
#include <Windows.h>

typedef int32_t bool32;
typedef int32_t i32;
typedef uint32_t u32;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define CELL_COUNT 30

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(*arr))
#define RGB(r, g, b) (float)(r)/255, (float)(g)/255, (float)(b)/255

enum Direction { Up = 0, Right, Down, Left };

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

struct TailPiece {
    glm::ivec2 pos;
};

struct SnakeData {
    std::deque<TailPiece> tail;
    glm::ivec2 velocity;
    bool32 should_grow;
    i32 dir;
    i32 length;
};

struct Vertex {
    glm::vec2 pos;
};

struct ObjectData {
    u32 vao;
    u32 vbo;
    u32 vertex_count;
    u32 shader;
    u32 primitive;
};

struct {
    bool32 pause;
    bool32 changed_direction_this_frame;
    glm::ivec2 food_pos;
    glm::ivec2 initial_positions[3];
    struct {
        i32 size;
        i32 data[3];
    } turns_queue;
} g;

void init_global_state() {
    g.pause = false;
    g.changed_direction_this_frame = false;
    g.food_pos = glm::ivec2(6, 5);
    g.turns_queue.size = 0;

    memset(g.turns_queue.data, 0, sizeof(g.turns_queue.data));
    g.turns_queue.data[0] = Right;

    g.initial_positions[0] = glm::ivec2(3, 1);
    g.initial_positions[1] = glm::ivec2(2, 1);
    g.initial_positions[2] = glm::ivec2(1, 1);
}

void restart_game(SnakeData *snake) {
    snake->length = ARR_SIZE(g.initial_positions);
    snake->tail.clear();

    for (i32 i = 0; i < ARR_SIZE(g.initial_positions); i++) {
        snake->tail.push_back({ g.initial_positions[i] });
    }

    snake->dir = Right;
    snake->should_grow = false;
    snake->velocity = glm::ivec2(1, 0);
}

void wait_until_next_frame(FramerateData *fr) {
    fr->current_time = glfwGetTime() * 1000;
    u32 time_taken = fr->current_time - fr->prev_time - fr->time_slept;
    fr->prev_time = fr->current_time;

    fr->time_slept = 1000 / fr->fps - time_taken;
    if (fr->time_slept < 0) {
        fr->time_slept = 0;
    }

    // pri32f("TIME TAKEN: %d\n", time_taken + fr->time_slept);

    Sleep(fr->time_slept);
}

void render_object(ObjectData *object) {
    glUseProgram(object->shader);
    glBindVertexArray(object->vao);
    glDrawArrays(object->primitive, 0, object->vertex_count);
    glBindVertexArray(0);
    glUseProgram(0);
}

void render_cell(ObjectData *quad, i32 x, i32 y) {
    glUseProgram(quad->shader);
    glUniform2f(glGetUniformLocation(quad->shader, "offset"), x, y);

    glBindVertexArray(quad->vao);
    glDrawElements(quad->primitive, quad->vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void render_food(ObjectData *quad) {
    glUseProgram(quad->shader);
    glUniform3f(glGetUniformLocation(quad->shader, "color"), 1.0f, 1.0f, 0.0f);
    render_cell(quad, g.food_pos.x, g.food_pos.y);
}

void render_snake(SnakeData *snake, ObjectData *quad) {
    glUseProgram(quad->shader);
    glUniform3f(glGetUniformLocation(quad->shader, "color"), 1.0f, 0.0f, 0.0f);
    auto head = snake->tail.begin();
    for (auto it = head + 1; it != snake->tail.end(); ++it) {
        render_cell(quad, it->pos.x, it->pos.y);
    }

    glUseProgram(quad->shader);
    glUniform3f(glGetUniformLocation(quad->shader, "color"), RGB(234, 221, 202));
    render_cell(quad, head->pos.x, head->pos.y);
}

bool32 can_change_direction(i32 old_dir, i32 new_dir) {
    bool32 result;
    if (old_dir == Left || old_dir == Right) {
        result = (new_dir == Up || new_dir == Down);
    } else {
        result = (new_dir == Left || new_dir == Right);
    }

    return result;
}

void key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_P) {
        g.pause = !g.pause;
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    SnakeData *snake = (SnakeData *)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_W) {
        snake->should_grow = true;
    }
    
    if (key >= GLFW_KEY_RIGHT && key <= GLFW_KEY_UP) {
        i32 direction;
        switch (key) {
            #define MAP_DIRECTION(key, dir) case key: direction = dir; break
            MAP_DIRECTION(GLFW_KEY_UP, Up);
            MAP_DIRECTION(GLFW_KEY_RIGHT, Right);
            MAP_DIRECTION(GLFW_KEY_DOWN, Down);
            MAP_DIRECTION(GLFW_KEY_LEFT, Left);
        }

        size_t turns_queue_capacity = ARR_SIZE(g.turns_queue.data);

        if (g.turns_queue.size < turns_queue_capacity) {
            g.turns_queue.data[g.turns_queue.size++] = direction;
        }
    }
}

void turn_snake(SnakeData *snake) {
    if (g.turns_queue.size > 0) {
        i32 new_dir = g.turns_queue.data[0];

        for (size_t i = 1; i < g.turns_queue.size; i++) {
            g.turns_queue.data[i - 1] = g.turns_queue.data[i];
        }

        g.turns_queue.size--;

        if (can_change_direction(snake->dir, new_dir) && !g.changed_direction_this_frame) {
            g.changed_direction_this_frame = true;
            switch (new_dir) {
                #define DIR_TO_VELOCITY(dir, x, y) case dir: snake->velocity = glm::ivec2(x, y); break
                DIR_TO_VELOCITY(Up, 0, -1);
                DIR_TO_VELOCITY(Right, 1, 0);
                DIR_TO_VELOCITY(Down, 0, 1);
                DIR_TO_VELOCITY(Left, -1, 0);
            }

            snake->dir = new_dir;
        }
    }
}

void update_snake(SnakeData *snake) {
    turn_snake(snake);
    snake->tail.push_front({ snake->tail[0].pos + snake->velocity });

    if (snake->should_grow) {
        snake->should_grow = false;
    } else {
        snake->tail.pop_back();
    }

    glm::ivec2 *head_pos = &snake->tail[0].pos;
    if (head_pos->x < 0)               head_pos->x = CELL_COUNT - 1;
    if (head_pos->y < 0)               head_pos->y = CELL_COUNT - 1;
    if (head_pos->x > CELL_COUNT - 1) head_pos->x = 0;
    if (head_pos->y > CELL_COUNT - 1) head_pos->y = 0;

    if (*head_pos == g.food_pos) {
        bool32 overlaps = false;
        glm::ivec2 new_food_pos;
        do {
            new_food_pos = glm::ivec2(rand() % CELL_COUNT, rand() % CELL_COUNT);

            for (const auto& tail_piece : snake->tail) {
                if (tail_piece.pos == new_food_pos) {
                    overlaps = true;
                    break;
                }
            }
        } while (overlaps);

        g.food_pos = new_food_pos;
        snake->should_grow = true;
    }

    auto head = snake->tail.begin();
    for (auto it = head + 1; it != snake->tail.end(); ++it) {
        if (head->pos == it->pos) {
            restart_game(snake);
            break;
        }
    }
}

i32 main() {
    init_global_state();
    srand(time(0));
    glfwInit();
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Snake Game", 0, 0);
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glfwSetKeyCallback(window, key_callback);

    float cell_width = (float)WINDOW_WIDTH / CELL_COUNT;
    float cell_height = (float)WINDOW_HEIGHT / CELL_COUNT;

    ObjectData grid;
    {
        const i32 grid_vertices_count = (CELL_COUNT - 1) * 4;
        Vertex grid_vertices[grid_vertices_count];

        for (size_t i = 0; i < CELL_COUNT - 1; i++) {
            float x = cell_width * (i + 1) / (float)WINDOW_WIDTH * 2 - 1;
            float y = cell_height * (i + 1) / (float)WINDOW_HEIGHT * 2 - 1;
            
            grid_vertices[i * 4 + 0] = { glm::vec2(x, 1) };
            grid_vertices[i * 4 + 1] = { glm::vec2(x, -1) };
            grid_vertices[i * 4 + 2] = { glm::vec2(-1, y) };
            grid_vertices[i * 4 + 3] = { glm::vec2(1, y) };
        }

        grid.vertex_count = grid_vertices_count;
        grid.primitive = GL_LINES;
        glGenBuffers(1, &grid.vbo);
        glGenVertexArrays(1, &grid.vao);

        glBindVertexArray(grid.vao);
        glBindBuffer(GL_ARRAY_BUFFER, grid.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), grid_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        u32 grid_shader = glCreateProgram();
        u32 grid_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        u32 grid_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        compile_shader_file(grid_vertex_shader, "./grid.vert");
        compile_shader_file(grid_fragment_shader, "./grid.frag");

        glAttachShader(grid_shader, grid_vertex_shader);
        glAttachShader(grid_shader, grid_fragment_shader);
        glLinkProgram(grid_shader);

        grid.shader = grid_shader;

        glDeleteShader(grid_vertex_shader);
        glDeleteShader(grid_fragment_shader);
    } // grid configuration

    float normalized_cell_width = cell_width / (float)WINDOW_WIDTH * 2.0f;
    float normalized_cell_height = cell_height / (float)WINDOW_HEIGHT * 2.0f;

    ObjectData quad;
    {
        Vertex quad_vertices[] = {
            { {-normalized_cell_width / 2, normalized_cell_height / 2} },
            { {-normalized_cell_width / 2, -normalized_cell_height / 2} },
            { {normalized_cell_width / 2, normalized_cell_height / 2} },
            { {normalized_cell_width / 2, -normalized_cell_height / 2} },

        };

        u32 quad_indices[] = {
            0, 1, 3,
            0, 2, 3
        };

        quad.vertex_count = ARR_SIZE(quad_indices);
        quad.primitive = GL_TRIANGLES;

        u32 ebo;
        glGenBuffers(1, &quad.vbo);
        glGenBuffers(1, &ebo);
        glGenVertexArrays(1, &quad.vao);
        glBindVertexArray(quad.vao);

        glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        quad.shader = glCreateProgram();
        u32 quad_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        u32 quad_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        compile_shader_file(quad_vertex_shader, "./quad.vert");
        compile_shader_file(quad_fragment_shader, "./quad.frag");

        glAttachShader(quad.shader, quad_vertex_shader);
        glAttachShader(quad.shader, quad_fragment_shader);
        glLinkProgram(quad.shader);

        glDeleteShader(quad_vertex_shader);
        glDeleteShader(quad_fragment_shader);

        glUseProgram(quad.shader);
        glUniform2f(glGetUniformLocation(quad.shader, "quad_size"), normalized_cell_width, normalized_cell_height);
        glUniform2f(glGetUniformLocation(quad.shader, "offset"), 2, 3);
        glUseProgram(0);
    } // quad configuration

    SnakeData snake;
    restart_game(&snake);
    glfwSetWindowUserPointer(window, &snake);

    FramerateData framerate(20);

    while (!glfwWindowShouldClose(window)) {
        g.changed_direction_this_frame = false;
        glfwPollEvents();

        if (g.pause) {
            Sleep(100);
            continue;
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        update_snake(&snake);

        render_food(&quad);
        render_snake(&snake, &quad);
        render_object(&grid);

        glfwSwapBuffers(window);
        wait_until_next_frame(&framerate);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
