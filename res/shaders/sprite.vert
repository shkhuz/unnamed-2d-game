#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texcoords;

uniform mat4 proj;
uniform mat4 view;

out vec4 fvertex_color;
out vec2 ftexcoords;

void main() {
   gl_Position = proj * view * vec4(position.xy, 0.0, 1.0);
   fvertex_color = color;
   ftexcoords = texcoords;
}