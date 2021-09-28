#ifndef _GRID_H_
#define _GRID_H_

#include "typedefs.h"
#include "util.h"

#include <glm/glm.hpp>

ObjectData configure_grid(glm::ivec2 window_size) {
    ObjectData grid;

    float cell_width = (float)window_size.x / CELL_COUNT;
    float cell_height = (float)window_size.y / CELL_COUNT;

    const i32 grid_vertices_count = (CELL_COUNT - 1) * 4 + 8;
    Vertex grid_vertices[grid_vertices_count];

    for (size_t i = 0; i < CELL_COUNT - 1; i++) {
        float x = cell_width * (i + 1) / (float)window_size.x * 2 - 1;
        float y = cell_height * (i + 1) / (float)window_size.y * 2 - 1;

        grid_vertices[i * 4 + 0] = { glm::vec2(x, 1) };
        grid_vertices[i * 4 + 1] = { glm::vec2(x, -1) };
        grid_vertices[i * 4 + 2] = { glm::vec2(-1, y) };
        grid_vertices[i * 4 + 3] = { glm::vec2(1, y) };
    }

    float limit = 0.999f;
    glm::vec2 top_left = glm::vec2(-limit, limit);
    glm::vec2 top_right = glm::vec2(limit, limit);
    glm::vec2 bottom_left = glm::vec2(-limit, -limit);
    glm::vec2 bottom_right = glm::vec2(limit, -limit);

    {
        i32 offset = 1;
        #define SET_BORDER_VERTICES(from, to) \
            grid_vertices[grid_vertices_count - offset++] = {(from)}; \
            grid_vertices[grid_vertices_count - offset++] = {(to)}

        SET_BORDER_VERTICES(top_left, top_right);
        SET_BORDER_VERTICES(top_right, bottom_right);
        SET_BORDER_VERTICES(bottom_right, bottom_left);
        SET_BORDER_VERTICES(bottom_left, top_left);
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

    compile_shader_file(grid_vertex_shader, "./shaders/grid.vert");
    compile_shader_file(grid_fragment_shader, "./shaders/grid.frag");

    glAttachShader(grid_shader, grid_vertex_shader);
    glAttachShader(grid_shader, grid_fragment_shader);
    glLinkProgram(grid_shader);

    grid.shader = grid_shader;

    glDeleteShader(grid_vertex_shader);
    glDeleteShader(grid_fragment_shader);

    return grid;
}

#endif
