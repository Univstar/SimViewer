#version 410 core

layout(location = 0) in  vec3  a_Position;
layout(location = 1) in  vec3  a_Normal;
layout(location = 2) in  float a_Heat;
layout(location = 3) in  float a_Radius;

layout(location = 0) out vec3  v_Position;
layout(location = 1) out vec3  v_Normal;
layout(location = 2) out float v_Heat;
layout(location = 3) out float v_Radius;

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
	v_Normal = a_Normal;
	v_Radius    = a_Radius;
	v_Heat      = a_Heat;
	gl_Position = u_Transform * vec4(v_Position, 1.);
}
