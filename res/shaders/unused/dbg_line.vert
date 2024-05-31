#version 330 core

layout (location = 0) in vec2 position;
out vec4 fvertex_color;

uniform mat4 proj;
uniform mat4 view;
uniform ivec2 chunk;

void main() {
   gl_Position = proj * view *
       vec4(position.x + chunk.x*32*16, position.y + chunk.y*32*16, 0.0, 1.0);
   fvertex_color = vec4(1.0, 1.0, 0.0, 1.0);
}
