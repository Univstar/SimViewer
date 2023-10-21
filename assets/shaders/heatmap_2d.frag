#version 410 core

layout(location = 0) in  vec2  v_Position;
layout(location = 1) in  float v_Heat;

layout(location = 0) out vec4 f_Color;

uniform float u_Scale;

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
    float heat = v_Heat > 0. ? v_Heat * u_Scale : 0.;
    heat = clamp(heat, 0., 1.);
	f_Color = vec4(GetJet(heat), 1.);
}
