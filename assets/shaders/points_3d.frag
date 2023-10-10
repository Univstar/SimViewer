#version 410 core

layout(location = 0) in  vec3 v_Position;
layout(location = 1) in  vec3 v_Normal;

layout(location = 0) out vec4 f_Color;

layout(std140) uniform PassConstants {
	mat4 u_Transform;
	vec3 u_LightIntensity;
	vec3 u_LightDirection;
	vec3 u_AmbientCoeff;
	vec3 u_CameraPosition;
	bool u_Wireframed;
	bool u_Flat;
};

uniform vec4  u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness*roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = roughness + 1.0;
	float k = r * r / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 Shade(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness) {
	vec3 halfDir = normalize(viewDir + lightDir);
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	float NDF = DistributionGGX(normal, halfDir, roughness);
	float G   = GeometrySmith(normal, viewDir, lightDir, roughness);
	vec3  F   = FresnelSchlick(clamp(dot(halfDir, viewDir), 0.0, 1.0), F0);

	vec3  numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
	vec3  specular = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	return (kD * albedo / PI + specular) * max(dot(normal, lightDir), 0.0);
}

void main() {
	vec3  normal    = normalize(v_Normal);
	vec3  viewDir   = normalize(u_CameraPosition - v_Position);
	if (dot(normal, viewDir) < 0) normal = -normal;
	// Maybe sample textures in the future
	vec3  albedo    = vec3(u_Albedo);
	float metallic  = u_Metallic;
	float roughness = u_Roughness;

	vec3 result = u_AmbientCoeff * albedo; // Ambient component
	result += Shade(u_LightDirection, normal, viewDir, albedo, metallic, roughness) * u_LightIntensity;

	result = u_Wireframed ? albedo : result;

	// HDR tonemapping
	result = result / (result + vec3(1.0));
	// Gamma correction
	result = pow(result, vec3(1. / 2.2));

	f_Color = vec4(result, 1.);
}
