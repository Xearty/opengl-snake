#ifndef _SNAKE_H_
#define _SNAKE_H_

#include "typedefs.h"
#include "quad.h"

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <deque>

struct TailPiece {
    glm::ivec2 pos;
};

struct SnakeData {
    std::deque<TailPiece> tail;
    glm::ivec2 velocity;
    bool32 should_grow;
};

struct TurnsQueue {
    i32 size;
    i32 data[3];
};

struct GameState {
    SnakeData snake;
    TurnsQueue turns_queue;
    i32 map[CELL_COUNT][CELL_COUNT];
    bool32 paused;
    i32 cells_left;
    glm::ivec2 food_pos;
    glm::ivec2 initial_positions[3];
};

void push_queue(TurnsQueue *queue, i32 key) {
    if (queue->size < ARR_SIZE(queue->data)) {
        queue->data[queue->size++] = key;
    }
}

static i32 pop_queue(TurnsQueue *queue) {
    if (queue->size <= 0) {
        return 0;
    }

    i32 new_dir = queue->data[0];
    for (size_t i = 1; i < queue->size; i++) {
        queue->data[i - 1] = queue->data[i];
    }

    queue->size--;
    return new_dir;
}

static bool32 can_change_direction(glm::ivec2 old_velocity, glm::ivec2 new_velocity) {
    glm::ivec2 sum = old_velocity + new_velocity;
    return sum.x && sum.y;
}

static glm::ivec2 gen_random_food_pos(i32 map[CELL_COUNT][CELL_COUNT]) {
    bool32 overlaps;
    glm::ivec2 new_food_pos;

    do {
        overlaps = false;
        new_food_pos = glm::ivec2(rand() % CELL_COUNT, rand() % CELL_COUNT);
        overlaps = map[new_food_pos.y][new_food_pos.x];
    } while (overlaps);

    return new_food_pos;
}

static void turn_snake(SnakeData *snake, TurnsQueue *queue) {
    i32 new_dir = pop_queue(queue);

    if (new_dir) {
        glm::ivec2 new_velocity;
        switch (new_dir) {
            #define KEY_TO_VELOCITY(key, x, y) case key: new_velocity = glm::ivec2(x, y); break
            KEY_TO_VELOCITY(GLFW_KEY_UP, 0, -1);
            KEY_TO_VELOCITY(GLFW_KEY_RIGHT, 1, 0);
            KEY_TO_VELOCITY(GLFW_KEY_DOWN, 0, 1);
            KEY_TO_VELOCITY(GLFW_KEY_LEFT, -1, 0);
        }

        if (can_change_direction(snake->velocity, new_velocity)) {
            snake->velocity = new_velocity;
        }
    }
}

void restart_game(GameState *game) {
    static const glm::ivec2 initial_positions[] = {
        {3, 1}, {2, 1}, {1, 1}
    };
    
    game->paused = false;
    game->turns_queue.size = 0;
    game->cells_left = CELL_COUNT * CELL_COUNT - ARR_SIZE(initial_positions);
    game->snake.tail.clear();
    memset(game->map, 0, sizeof(game->map));

    for (i32 i = 0; i < ARR_SIZE(initial_positions); i++) {
        game->snake.tail.push_back({ initial_positions[i] });
        game->map[initial_positions[i].y][initial_positions[i].x] = 1;
    }

    game->snake.should_grow = false;
    game->snake.velocity = glm::ivec2(1, 0);

    game->food_pos = gen_random_food_pos(game->map);
}

void update_snake(GameState *game) {
    SnakeData *snake = &game->snake;
    TurnsQueue *queue = &game->turns_queue;
    turn_snake(snake, queue);

    if (snake->should_grow) {
        snake->should_grow = false;

        if (--game->cells_left <= 0) {
            game->paused = true;
            return;
        }
    } else {
        glm::ivec2 tail_tip_pos = snake->tail.back().pos;
        game->map[tail_tip_pos.y][tail_tip_pos.x] = 0;
        snake->tail.pop_back();
    }

    snake->tail.push_front({ snake->tail.front().pos + snake->velocity });

    glm::ivec2 *head_pos = &snake->tail.front().pos;
    if (head_pos->x < 0)              head_pos->x = CELL_COUNT - 1;
    if (head_pos->y < 0)              head_pos->y = CELL_COUNT - 1;
    if (head_pos->x > CELL_COUNT - 1) head_pos->x = 0;
    if (head_pos->y > CELL_COUNT - 1) head_pos->y = 0;

    if (game->map[head_pos->y][head_pos->x]) {
        restart_game(game);
        return;
    }

    game->map[head_pos->y][head_pos->x] = 1;

    if (*head_pos == game->food_pos) {
        game->food_pos = gen_random_food_pos(game->map);
        snake->should_grow = true;
    }
}

void render_cell(ObjectData *quad, i32 x, i32 y) {
    glUniform2f(glGetUniformLocation(quad->shader, "offset"), x, y);

    glBindVertexArray(quad->vao);
    glDrawElements(quad->primitive, quad->vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void render_food(ObjectData *quad, glm::ivec2 food_pos) {
    glUseProgram(quad->shader);
    glUniform3f(glGetUniformLocation(quad->shader, "color"), 1.0f, 1.0f, 0.0f);
    render_cell(quad, food_pos.x, food_pos.y);
}

void render_snake(SnakeData *snake, ObjectData *quad) {
    glUseProgram(quad->shader);
    glUniform3f(glGetUniformLocation(quad->shader, "color"), RGB_VAL(234, 221, 202));
    auto head = snake->tail.begin();
    render_cell(quad, head->pos.x, head->pos.y);

    glUniform3f(glGetUniformLocation(quad->shader, "color"), 1.0f, 0.0f, 0.0f);
    for (auto it = head + 1; it != snake->tail.end(); ++it) {
        render_cell(quad, it->pos.x, it->pos.y);
    }

    glUseProgram(0);
}

#endif
