#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 8) out;

layout(location = 0) in  vec3  v_Position[];
layout(location = 1) in  vec3 v_Normal[];
layout(location = 2) in  float v_Heat[];
layout(location = 3) in  float v_Radius[];

layout(location = 0) out vec3  g_Position;
layout(location = 1) out vec3 g_Normal;
layout(location = 2) out vec3  g_TexCoord;
layout(location = 3) out float g_Heat;

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
	g_Position  = v_Position[0] + vec3(-v_Radius[0], -v_Radius[0], -v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(0., 0., 0.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();

	g_Position  = v_Position[0] + vec3( v_Radius[0], -v_Radius[0], -v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(1., 0., 0.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();
	
	g_Position  = v_Position[0] + vec3(-v_Radius[0],  v_Radius[0], -v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(0., 1., 0.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();

	g_Position  = v_Position[0] + vec3( v_Radius[0],  v_Radius[0], -v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(1., 1., 0.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();

	g_Position  = v_Position[0] + vec3(-v_Radius[0], -v_Radius[0], v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(0., 0., 1.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();

	g_Position  = v_Position[0] + vec3( v_Radius[0], -v_Radius[0], v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(1., 0., 1.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();
	
	g_Position  = v_Position[0] + vec3(-v_Radius[0],  v_Radius[0], v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(0., 1., 1.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();

	g_Position  = v_Position[0] + vec3( v_Radius[0],  v_Radius[0], v_Radius[0]) * u_RadScale;
	g_Normal = v_Normal[0];
	g_TexCoord  = vec3(1., 1., 1.);
	g_Heat	    = v_Heat[0];
	gl_Position = u_Transform * vec4(g_Position, 1.);
	EmitVertex();

	EndPrimitive();
}
