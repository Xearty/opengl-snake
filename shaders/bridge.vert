#version 330 core

layout (location = 0) in vec2 position;

uniform vec2 cell_size;
uniform ivec2 cell_position;
uniform vec2 offset;
uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(position + cell_size / 2 + cell_size * cell_position + offset, 1.0f, 1.0f) * vec4(1.0f, -1.0f, 1.0f, 1.0f);
}
