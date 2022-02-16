#ifndef _CELL_H_
#define _CELL_H_

#include "typedefs.h"
#include "util.h"

#include <stddef.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

ObjectData configure_cell(glm::ivec2 window_size) {
    ObjectData cell;

    float cell_width = (float)window_size.y / CELL_COUNT;
    float cell_height = (float)window_size.y / CELL_COUNT;

    Vertex cell_vertices[] = {
        { {-cell_width / 2 + GAP,  cell_height / 2 - GAP} },
        { {-cell_width / 2 + GAP, -cell_height / 2 + GAP} },
        { { cell_width / 2 - GAP,  cell_height / 2 - GAP} },
        { { cell_width / 2 - GAP, -cell_height / 2 + GAP} },

    };

    u32 cell_indices[] = {
        0, 1, 3,
        0, 2, 3
    };

    cell.vertex_count = ARR_SIZE(cell_indices);
    cell.primitive = GL_TRIANGLES;

    u32 ebo;
    glGenBuffers(1, &cell.vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &cell.vao);
    glBindVertexArray(cell.vao);

    glBindBuffer(GL_ARRAY_BUFFER, cell.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cell_vertices), cell_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cell_indices), cell_indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    cell.shader = glCreateProgram();
    u32 cell_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    u32 cell_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    compile_shader_file(cell_vertex_shader, "./shaders/cell.vert");
    compile_shader_file(cell_fragment_shader, "./shaders/cell.frag");

    glAttachShader(cell.shader, cell_vertex_shader);
    glAttachShader(cell.shader, cell_fragment_shader);
    glLinkProgram(cell.shader);

    glDeleteShader(cell_vertex_shader);
    glDeleteShader(cell_fragment_shader);

    glUseProgram(cell.shader);
    glUniform2f(glGetUniformLocation(cell.shader, "cell_size"), cell_width, cell_height);
    glUniform2f(glGetUniformLocation(cell.shader, "offset"), 2, 3);

    glm::mat4 projection = glm::ortho(0.0f, (float)window_size.y, 0.0f, (float)window_size.y);
    glUniformMatrix4fv(glGetUniformLocation(cell.shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(0);

    return cell;
}

#endif
