#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texcoords;

uniform mat4 proj;
uniform mat4 view;
uniform float time;

out vec4 fcolor;
out vec2 ftexcoords;

float speed = 0.5;
float min_strength = 0.05;
float max_strength = 0.01;
float strength_scale = 1.0;
float interval = 3.5;
float detail = 1.0;
float distortion = 0.5;
float height_offset = 0.0;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float get_wind(vec2 vertex, vec2 uv, float time) {
    float diff = pow(max_strength - min_strength, 2.0);
    float strength = clamp(min_strength + diff + sin(time / interval) * diff, min_strength, max_strength) * strength_scale;
    float wind = (sin(time) + cos(time * detail)) * strength * max(0.0, (1.0-uv.y) - height_offset);
    return wind;
}

void main() {
    gl_Position = proj * view * vec4(position, 0.0, 1.0);
    if (gl_VertexID%4 == 0 || gl_VertexID%4 == 1) {
                                                            // Using (time*speed) + gl_VertexID) creates jitter
        gl_Position.x += get_wind(gl_Position.xy, texcoords, (time*speed) + position.x + position.y);
    }
    fcolor = color;
    ftexcoords = texcoords;
}
