#ifndef _MY_TYPES_H_
#define _MY_TYPES_H_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdint.h>

typedef int32_t bool32;
typedef int32_t i32;
typedef uint32_t u32;

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

void render_object(ObjectData *object) {
    glUseProgram(object->shader);
    glBindVertexArray(object->vao);
    glDrawArrays(object->primitive, 0, object->vertex_count);
    glBindVertexArray(0);
    glUseProgram(0);
}

#endif
