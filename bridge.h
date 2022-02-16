#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include "typedefs.h"
#include "util.h"

#include <stddef.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

enum { HORIZONTAL, VERTICAL };
static i32 last_rotation = HORIZONTAL;

void render_bridge(ObjectData *bridge, glm::vec2 cell_size, glm::ivec2 position, glm::ivec2 direction) {
    if (direction.x ==  CELL_COUNT - 1) direction.x = -1;
    if (direction.x == -CELL_COUNT + 1) direction.x =  1;
    if (direction.y ==  CELL_COUNT - 1) direction.y = -1;
    if (direction.y == -CELL_COUNT + 1) direction.y =  1;

    glUseProgram(bridge->shader);
    glUniform2i(glGetUniformLocation(bridge->shader, "cell_position"), position.x, position.y);
    glBindVertexArray(bridge->vao);

    if (direction.x == 0) {
        glUniform2f(glGetUniformLocation(bridge->shader, "offset"), 0.0f, (cell_size.y / 2 - GAP / 2) * direction.y);
        if (last_rotation != HORIZONTAL) {
            Vertex bridge_vertices[] = {
                { {-cell_size.x / 2 + GAP, -GAP / 2} },
                { {-cell_size.x / 2 + GAP,  GAP / 2} },
                { { cell_size.x / 2 - GAP, -GAP / 2} },
                { { cell_size.x / 2 - GAP,  GAP / 2} },
            };

            glBindBuffer(GL_ARRAY_BUFFER, bridge->vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bridge_vertices), bridge_vertices);
            last_rotation = HORIZONTAL;
        }
    } else {
        glUniform2f(glGetUniformLocation(bridge->shader, "offset"), (cell_size.x / 2 - GAP / 2) * direction.x, 0.0f);
        if (last_rotation != VERTICAL) {
            Vertex bridge_vertices[] = {
                { {-GAP / 2, -cell_size.y / 2 + GAP} },
                { {-GAP / 2,  cell_size.y / 2 - GAP} },
                { { GAP / 2, -cell_size.y / 2 + GAP} },
                { { GAP / 2,  cell_size.y / 2 - GAP} },
            };

            glBindBuffer(GL_ARRAY_BUFFER, bridge->vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bridge_vertices), bridge_vertices);
            last_rotation = VERTICAL;
        }

    }

    glDrawElements(bridge->primitive, bridge->vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

ObjectData configure_bridge(glm::ivec2 window_size) {
    ObjectData bridge;

    float cell_width = (float)window_size.y / CELL_COUNT;
    float cell_height = (float)window_size.y / CELL_COUNT;

    Vertex bridge_vertices[] = {
        { {-cell_width / 2 + GAP, -GAP / 2} },
        { {-cell_width / 2 + GAP,  GAP / 2} },
        { { cell_width / 2 - GAP, -GAP / 2} },
        { { cell_width / 2 - GAP,  GAP / 2} },

    };

    u32 bridge_indices[] = {
        0, 1, 3,
        0, 2, 3
    };

    bridge.vertex_count = ARR_SIZE(bridge_indices);
    bridge.primitive = GL_TRIANGLES;

    u32 ebo;
    glGenBuffers(1, &bridge.vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &bridge.vao);
    glBindVertexArray(bridge.vao);

    glBindBuffer(GL_ARRAY_BUFFER, bridge.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bridge_vertices), bridge_vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bridge_indices), bridge_indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    bridge.shader = glCreateProgram();
    u32 bridge_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    u32 bridge_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    compile_shader_file(bridge_vertex_shader, "./shaders/bridge.vert");
    compile_shader_file(bridge_fragment_shader, "./shaders/bridge.frag");

    glAttachShader(bridge.shader, bridge_vertex_shader);
    glAttachShader(bridge.shader, bridge_fragment_shader);
    glLinkProgram(bridge.shader);

    glDeleteShader(bridge_vertex_shader);
    glDeleteShader(bridge_fragment_shader);

    glUseProgram(bridge.shader);
    glUniform2f(glGetUniformLocation(bridge.shader, "cell_size"), cell_width, cell_height);
    glUniform2f(glGetUniformLocation(bridge.shader, "cell_position"), 2, 3);

    glm::mat4 projection = glm::ortho(0.0f, (float)window_size.y, 0.0f, (float)window_size.y);
    glUniformMatrix4fv(glGetUniformLocation(bridge.shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(0);

    return bridge;
}

#endif
