#ifndef _SNAKE_H_
#define _SNAKE_H_

#include "typedefs.h"
#include "quad.h"

#include <glad/glad.h>
#include <deque>

struct TailPiece {
    Vec2i pos;
};

struct SnakeData {
    std::deque<TailPiece> tail;
    Vec2i velocity;
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
    bool32 is_over;
    i32 cells_left;
    Vec2i food_pos;
};

static void map_set(i32 map[CELL_COUNT][CELL_COUNT], Vec2i pos, i32 value) {
    map[pos.y][pos.x] = value;
}

static i32 map_at(i32 map[CELL_COUNT][CELL_COUNT], Vec2i pos) {
    return map[pos.y][pos.x];
}

void push_queue(TurnsQueue *queue, i32 key) {
    if (queue->size < ARR_SIZE(queue->data)) {
        queue->data[queue->size++] = key;
    }
}

static i32 pop_queue(TurnsQueue *queue) {
    i32 result = 0;

    if (queue->size > 0) {
        result = queue->data[0];
        for (size_t i = 1; i < queue->size; i++) {
            queue->data[i - 1] = queue->data[i];
        }
        queue->size--;
    }

    return result;
}

static void push_new_head(std::deque<TailPiece> *tail, i32 map[CELL_COUNT][CELL_COUNT], Vec2i pos) {
    tail->push_front({ pos });
    map_set(map, pos, 1);
}

static void pop_tail(std::deque<TailPiece> *tail, i32 map[CELL_COUNT][CELL_COUNT]) {
    Vec2i tail_tip_pos = tail->back().pos;
    tail->pop_back();
    map_set(map, tail_tip_pos, 0);
}

static bool32 can_change_direction(Vec2i old_velocity, Vec2i new_velocity) {
    Vec2i sum = old_velocity + new_velocity;
    return sum.x && sum.y;
}

static Vec2i gen_random_food_pos(i32 map[CELL_COUNT][CELL_COUNT]) {
    bool32 overlaps;
    Vec2i new_food_pos;

    do {
        overlaps = false;
        new_food_pos = { rand() % CELL_COUNT, rand() % CELL_COUNT };
        overlaps = map_at(map, new_food_pos);
    } while (overlaps);

    return new_food_pos;
}

static void turn_snake(SnakeData *snake, TurnsQueue *queue) {
    i32 new_dir = pop_queue(queue);

    if (new_dir) {
        Vec2i new_velocity;
        switch (new_dir) {
            #define KEY_TO_VELOCITY(key, x, y) case key: new_velocity = {x, y}; break
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
    const Vec2i initial_positions[] = {
        {3, 1}, {2, 1}, {1, 1}
    };
    
    game->is_over = false;
    game->paused = false;
    game->turns_queue.size = 0;
    game->cells_left = CELL_COUNT * CELL_COUNT - ARR_SIZE(initial_positions);
    game->snake.tail.clear();
    memset(game->map, 0, sizeof(game->map));

    for (i32 i = 0; i < ARR_SIZE(initial_positions); i++) {
        game->snake.tail.push_back({ initial_positions[i] });
        map_set(game->map, initial_positions[i], 1);
    }

    game->snake.should_grow = false;
    game->snake.velocity = { 1, 0 };

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
            game->is_over = true;
        }
    } else {
        pop_tail(&snake->tail, game->map);
    }

    Vec2i new_head_pos = snake->tail.front().pos + snake->velocity;

    if (new_head_pos.x < 0)              new_head_pos.x = CELL_COUNT - 1;
    if (new_head_pos.y < 0)              new_head_pos.y = CELL_COUNT - 1;
    if (new_head_pos.x > CELL_COUNT - 1) new_head_pos.x = 0;
    if (new_head_pos.y > CELL_COUNT - 1) new_head_pos.y = 0;

    if (!map_at(game->map, new_head_pos)) {
        push_new_head(&snake->tail, game->map, new_head_pos);
    } else {
        restart_game(game);
    }

    if (!game->is_over && new_head_pos == game->food_pos) {
        game->food_pos = gen_random_food_pos(game->map);
        snake->should_grow = true;
    }
}

void render_cell(ObjectData *quad, i32 x, i32 y) {
    glUniform2i(glGetUniformLocation(quad->shader, "offset"), x, y);

    glBindVertexArray(quad->vao);
    glDrawElements(quad->primitive, quad->vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void render_food(ObjectData *quad, Vec2i food_pos) {
    glUseProgram(quad->shader);
    glUniform3f(glGetUniformLocation(quad->shader, "color"), 1.0f, 1.0f, 0.0f);
    render_cell(quad, food_pos.x, food_pos.y);
}

void render_snake(std::deque<TailPiece> *tail, ObjectData *quad) {
    glUseProgram(quad->shader);
    glUniform3f(glGetUniformLocation(quad->shader, "color"), 235.0f / 255, 221.0f / 255, 202.0f / 255);
    auto head = tail->begin();
    render_cell(quad, head->pos.x, head->pos.y);

    glUniform3f(glGetUniformLocation(quad->shader, "color"), 1.0f, 0.0f, 0.0f);
    for (auto it = head + 1; it != tail->end(); ++it) {
        render_cell(quad, it->pos.x, it->pos.y);
    }

    glUseProgram(0);
}

#endif
