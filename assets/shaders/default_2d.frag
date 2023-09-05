#version 410 core

layout(location = 0) in  vec2 g_Position;
layout(location = 1) in  vec2 g_TexCoord;

layout(location = 0) out vec4 f_Color;

uniform vec4 u_Albedo;

void main() {
	f_Color = u_Albedo;
}
