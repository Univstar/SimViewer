#version 410 core

layout(location = 0) in  vec3  g_Position;
layout(location = 1) in  vec3  g_Normal;
layout(location = 2) in  vec3  g_TexCoord;
layout(location = 3) in  float g_Heat;

layout(location = 0) out vec4  f_Color;

uniform float u_HeatBias;
uniform float u_HeatScale;

float GetJetBase(float val) {
	if (val <= .125) {
        return 0.;
    } else if (val <= .375) {
        return 4. * val - .5;
    } else if (val <= .625) {
        return 1.;
    } else if (val <= .875) {
        return 3.5 - 4. * val;
    } else {
        return 0.;
    }
}

vec3 GetJet(float heat) {
	return vec3(GetJetBase(heat - .25), GetJetBase(heat), GetJetBase(heat + .25));
}

void main() {
	float dist = distance(g_TexCoord, vec3(0.5, 0.5, 0.5));
	if (dist < 1.) {
		float heat = u_HeatScale > 0 ? (g_Heat + u_HeatBias) * u_HeatScale : .5;
		f_Color = vec4(GetJet(heat), mix(1., 0., smoothstep(.618034, 1., dist)));
	} else {
		discard;
	}
}
