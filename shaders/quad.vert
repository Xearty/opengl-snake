#version 330 core

layout (location = 0) in vec2 position;

uniform vec2 quad_size;
uniform vec2 offset;

void main() {
	gl_Position = vec4(position.x + quad_size.x / 2 + offset.x * quad_size.x - 1,
					   -(position.y + quad_size.y / 2 + offset.y * quad_size.y) + 1,
					   1.0f,
					   1.0f);
}