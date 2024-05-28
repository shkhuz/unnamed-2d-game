#version 330 core

in vec4 fvertex_color;
in vec2 ftexcoords;

uniform sampler2D tex;

out vec4 out_color;

void main() {
    out_color = texture(tex, ftexcoords);
}
