#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in  vec2  v_Position[];
layout(location = 1) in  vec2  v_TexCoord[];
layout(location = 2) in  float v_Radius[];

layout(location = 0) out vec2 g_Position;
layout(location = 1) out vec2 g_TexCoord;

layout(std140) uniform PassConstants {
	mat4 u_Transform;
	vec3 u_LightIntensity;
	vec3 u_LightDirection;
	vec3 u_AmbientCoeff;
	vec3 u_CameraPosition;
	bool u_Wireframed;
	bool u_Flat;
};

uniform float u_RadScale;

void main() {
	g_Position  = v_Position[0] + vec2(-v_Radius[0], -v_Radius[0]) * u_RadScale;
	g_TexCoord  = vec2(0., 0.);
	gl_Position = u_Transform * vec4(g_Position, 0., 1.);
	EmitVertex();

	g_Position  = v_Position[0] + vec2( v_Radius[0], -v_Radius[0]) * u_RadScale;
	g_TexCoord  = vec2(1., 0.);
	gl_Position = u_Transform * vec4(g_Position, 0., 1.);
	EmitVertex();
	
	g_Position  = v_Position[0] + vec2(-v_Radius[0],  v_Radius[0]) * u_RadScale;
	g_TexCoord  = vec2(0., 1.);
	gl_Position = u_Transform * vec4(g_Position, 0., 1.);
	EmitVertex();

	g_Position  = v_Position[0] + vec2( v_Radius[0],  v_Radius[0]) * u_RadScale;
	g_TexCoord  = vec2(1., 1.);
	gl_Position = u_Transform * vec4(g_Position, 0., 1.);
	EmitVertex();

	EndPrimitive();
}
