#version 410 core

layout(location = 0) in  vec2 g_Position;
layout(location = 1) in  vec2 g_TexCoord;

layout(location = 0) out vec4 f_Color;

uniform vec4 u_Albedo;

void main() {
	vec2 delta = g_TexCoord - vec2(0.5, 0.5);
	if (delta.x * delta.x + delta.y * delta.y <= 0.25) {
		f_Color = u_Albedo;
	} else {
		discard;
	}
}
