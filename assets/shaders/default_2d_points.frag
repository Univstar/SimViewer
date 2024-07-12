#version 410 core

layout(location = 0) in  vec2 g_Position;
layout(location = 1) in  vec2 g_TexCoord;

layout(location = 0) out vec4 f_Color;

uniform vec4 u_Albedo;

void main() {
	float dist = distance(g_TexCoord, vec2(0.5, 0.5)) * 2.;
	if (dist < 1.) {
		f_Color = mix(u_Albedo, vec4(0., 0., 0., 1.), smoothstep(.618034, 1., dist));
	} else {
		discard;
	}
}
