#ifndef _QUAD_H_
#define _QUAD_H_

#include "typedefs.h"
#include "util.h"

#include <glm/glm.hpp>

ObjectData configure_quad(glm::ivec2 window_size) {
    ObjectData quad;

    float cell_width = (float)window_size.x / CELL_COUNT;
    float cell_height = (float)window_size.y / CELL_COUNT;

    float normalized_cell_width = cell_width / (float)window_size.x * 2.0f;
    float normalized_cell_height = cell_height / (float)window_size.y * 2.0f;

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

    compile_shader_file(quad_vertex_shader, "./shaders/quad.vert");
    compile_shader_file(quad_fragment_shader, "./shaders/quad.frag");

    glAttachShader(quad.shader, quad_vertex_shader);
    glAttachShader(quad.shader, quad_fragment_shader);
    glLinkProgram(quad.shader);

    glDeleteShader(quad_vertex_shader);
    glDeleteShader(quad_fragment_shader);

    glUseProgram(quad.shader);
    glUniform2f(glGetUniformLocation(quad.shader, "quad_size"), normalized_cell_width, normalized_cell_height);
    glUniform2f(glGetUniformLocation(quad.shader, "offset"), 2, 3);
    glUseProgram(0);

    return quad;
}

#endif
