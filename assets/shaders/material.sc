#ifndef MATERIAL_HEADER
#define MATERIAL_HEADER

#include <bgfx_shader.sh>

uniform vec4 u_baseColor;
uniform vec4 u_materialParams;
uniform vec4 u_emissive;

uniform vec4 u_lightDir;
uniform vec4 u_lightColor;
uniform vec4 u_ambientColor;

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_texNormal, 1);
SAMPLER2D(s_texMetallicRoughness, 2);
SAMPLER2D(s_texEmissive, 3);

struct Material
{
    vec4 baseColor;
    float metallic;
    float roughness;
    vec3 normal;
    vec3 emissive;

    vec3 diffuse;
    vec3 F0;
    float a;
};

const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
const vec3 black = vec3(0.0, 0.0, 0.0);
const float PI = 3.14159265359;
const float INV_PI = 0.31830988618;

Material initMaterial(Material mat)
{
    mat.diffuse = mix(mat.baseColor.rgb * (vec3(1.0, 1.0, 1.0) - dielectricSpecular),
                      black,
                      mat.metallic);

    mat.F0 = mix(dielectricSpecular, mat.baseColor.rgb, mat.metallic);

    mat.a = mat.roughness * mat.roughness;
    mat.a = max(mat.a, 0.01); // Prevent division by zero

    return mat;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (vec3(1.0, 1.0, 1.0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float ggxNDF(float NdH, float a)
{
    float a2 = a * a;
    float denom = (NdH * NdH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float schlickGGX(float NdV, float k)
{
    float nom = NdV;
    float denom = NdV * (1.0 - k) + k;
    return nom / max(denom, 0.001);
}

float geometrySmith(float NdV, float NdL, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return schlickGGX(NdV, k) * schlickGGX(NdL, k);
}

vec3 cookTorrance(vec3 N, vec3 V, vec3 L, Material mat)
{
    vec3 H = normalize(V + L);

    float NdV = max(dot(N, V), 0.0);
    float NdL = max(dot(N, L), 0.0);
    float NdH = max(dot(N, H), 0.0);
    float VdH = max(dot(V, H), 0.0);

    float NDF = ggxNDF(NdH, mat.a);
    vec3 F = fresnelSchlick(VdH, mat.F0);
    float G = geometrySmith(NdV, NdL, mat.roughness);

    vec3 kS = F;
    vec3 kD = (vec3(1.0, 1.0, 1.0) - kS) * (1.0 - mat.metallic);

    vec3 diffuse = kD * mat.baseColor.rgb * INV_PI;
    vec3 specular = (NDF * F * G) / max(4.0 * NdV * NdL, 0.001);

    return (diffuse + specular) * NdL;
}

#endif
