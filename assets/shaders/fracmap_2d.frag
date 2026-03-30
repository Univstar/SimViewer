#version 410 core

layout(location = 0) in  vec2  v_Position;
layout(location = 1) in  float v_Fraction;

layout(location = 0) out vec4 f_Color;

uniform vec4 u_Albedo;

void main() {
    f_Color = u_Albedo * v_Fraction;
}
