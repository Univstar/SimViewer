#version 410 core

layout(location = 0) in  vec2  a_Position;
layout(location = 1) in  float a_Fraction;

layout(location = 0) out vec2  v_Position;
layout(location = 1) out float v_Fraction;

layout(std140) uniform PassConstants {
	mat4 u_Transform;
	vec3 u_LightIntensity;
	vec3 u_LightDirection;
	vec3 u_AmbientCoeff;
	vec3 u_CameraPosition;
	bool u_Wireframed;
	bool u_Flat;
};

void main() {
	v_Position  = a_Position;
	v_Fraction  = a_Fraction;
	gl_Position = u_Transform * vec4(v_Position, 0., 1.);
}
