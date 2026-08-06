#ifndef MATERIAL_HEADER
#define MATERIAL_HEADER

#include <bgfx_shader.sh>

#define MAX_LIGHTS 16

uniform vec4 u_baseColor;
uniform vec4 u_materialParams;
uniform vec4 u_emissive;
uniform vec4 u_ambientColor;
uniform vec4 u_camPos;
uniform vec4 u_groundColor;
uniform vec4 u_envSkyColor;
uniform vec4 u_envSunDir;
uniform vec4 u_envSunColor;

uniform vec4 u_lightsPosType[MAX_LIGHTS];
uniform vec4 u_lightsDirRange[MAX_LIGHTS];
uniform vec4 u_lightsColorInt[MAX_LIGHTS];
uniform vec4 u_lightsCone[MAX_LIGHTS];
uniform vec4 u_lightsCount;

uniform vec4 u_shadowParams;
uniform vec4 u_shadowParams2;
uniform vec4 u_texFlags;
uniform vec4 u_iblParams;

#define MAX_CASCADES 4
uniform mat4 u_cascadeViewProj[MAX_CASCADES];
uniform vec4 u_cascadeOffsets[MAX_CASCADES];
uniform vec4 u_cascadeSplits;

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_texNormal, 1);
SAMPLER2D(s_texMetallicRoughness, 2);
SAMPLER2D(s_texEmissive, 3);
SAMPLER2D(s_shadowMap, 4);
SAMPLER2D(s_envMap, 5);

struct Material
{
    vec4 baseColor;
    float metallic;
    float roughness;
    vec3 normal;
    float occlusion;
    vec3 emissive;

    vec3 diffuse;
    vec3 F0;
    float a;
};

const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
const vec3 black = vec3(0.0, 0.0, 0.0);
const float INV_PI = 0.31830988618;

vec3 safeNormalize(vec3 v, vec3 fallback)
{
    float len2 = dot(v, v);
    if (len2 <= 0.000001)
        return fallback;

    return v * inversesqrt(len2);
}

float sanitizeScalar(float x, float maxValue)
{
    if (x != x)
        return 0.0;

    return clamp(x, 0.0, maxValue);
}

vec3 sanitizeColor(vec3 c, float maxValue)
{
    return vec3(
        sanitizeScalar(c.x, maxValue),
        sanitizeScalar(c.y, maxValue),
        sanitizeScalar(c.z, maxValue)
    );
}

vec3 srgbToLinear(vec3 c)
{
    c = clamp(c, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));
    vec3 lo = c / 12.92;
    vec3 hi_base = max((c + vec3(0.055, 0.055, 0.055)) / 1.055,
                       vec3(0.0, 0.0, 0.0));
    vec3 hi = pow(hi_base, vec3(2.4, 2.4, 2.4));
    vec3 use_hi = step(vec3(0.04045, 0.04045, 0.04045), c);
    return mix(lo, hi, use_hi);
}

Material initMaterial(Material mat)
{
    mat.occlusion = 1.0;
    mat.diffuse = mix(mat.baseColor.rgb * (vec3(1.0, 1.0, 1.0) - dielectricSpecular), black, mat.metallic);
    mat.F0 = mix(dielectricSpecular, mat.baseColor.rgb, mat.metallic);
    mat.roughness = max(mat.roughness, 0.045);
    mat.a = mat.roughness * mat.roughness;
    mat.a = max(mat.a, 0.01);
    return mat;
}

float specularAntiAliasing(vec3 N, float a)
{
    const float SIGMA2 = 0.25;
    const float KAPPA  = 0.18;

    vec3 dndu = dFdx(N);
    vec3 dndv = dFdy(N);
    float variance = SIGMA2 * (dot(dndu, dndu) + dot(dndv, dndv));
    float kernelRoughness2 = min(2.0 * variance, KAPPA);
    return saturate(a + kernelRoughness2);
}

vec3 F_Schlick(float VdH, vec3 F0)
{
    float f = pow(1.0 - VdH, 5.0);
    return f + F0 * (1.0 - f);
}

float D_GGX(float NdH, float a)
{
    a = NdH * a;
    float k = a / (1.0 - NdH * NdH + a * a);
    return k * k * INV_PI;
}

float V_SmithGGXCorrelated(float NdV, float NdL, float a)
{
    float a2 = a * a;
    float GGXV = NdL * sqrt(NdV * NdV * (1.0 - a2) + a2);
    float GGXL = NdV * sqrt(NdL * NdL * (1.0 - a2) + a2);
    return 0.5 / max(GGXV + GGXL, 0.0001);
}

float Fd_Lambert()
{
    return 1.0;
}

vec3 BRDF(vec3 V, vec3 L, vec3 N, float NdV, float NdL, Material mat)
{
    vec3 H = safeNormalize(L + V, N);
    float NdH = saturate(dot(N, H));
    float VdH = saturate(dot(V, H));

    float D = D_GGX(NdH, mat.a);
    vec3 F = F_Schlick(VdH, mat.F0);
    float Vis = V_SmithGGXCorrelated(NdV, NdL, mat.a);
    float specular_term = sanitizeScalar(Vis * D, 64.0);
    vec3 Fr = sanitizeColor(F * specular_term, 64.0);

    vec3 Fd = mat.diffuse * Fd_Lambert();
    vec3 diffuse_visibility = vec3(1.0, 1.0, 1.0) - F * 0.1;
    return sanitizeColor(Fr + diffuse_visibility * Fd, 4096.0);
}

struct LightData
{
    vec3 radiance;
    vec3 direction;
};

float distanceAttenuation(float dist)
{
    return 1.0 / max(dist * dist, 0.01 * 0.01);
}

float smoothAttenuation(float dist, float range)
{
    float safeRange = max(range, 0.0001);
    float ratio = dist / safeRange;
    float ratio2 = ratio * ratio;
    float window = saturate(1.0 - ratio2 * ratio2);
    return window * window * distanceAttenuation(dist);
}

LightData emptyLightData(vec3 fallbackDir)
{
    LightData data;
    data.radiance = vec3(0.0, 0.0, 0.0);
    data.direction = fallbackDir;
    return data;
}

LightData calcDirectionalLight(vec4 dirRange, vec4 colorInt)
{
    LightData data;
    data.direction = safeNormalize(-dirRange.xyz, vec3(0.0, 1.0, 0.0));
    data.radiance = sanitizeColor(colorInt.rgb * colorInt.w, 4096.0);
    return data;
}

LightData calcPointLight(vec3 worldPos, vec4 posType, vec4 dirRange, vec4 colorInt)
{
    vec3 toLight = posType.xyz - worldPos;
    float dist2 = dot(toLight, toLight);
    if (dist2 <= 0.000001)
        return emptyLightData(vec3(0.0, 1.0, 0.0));

    float dist = sqrt(dist2);

    LightData data;
    data.direction = toLight / dist;
    float attenuation = smoothAttenuation(dist, dirRange.w);
    data.radiance = attenuation > 0.0
        ? sanitizeColor(colorInt.rgb * colorInt.w * attenuation, 4096.0)
        : vec3(0.0, 0.0, 0.0);
    return data;
}

LightData calcSpotLight(vec3 worldPos, vec4 posType, vec4 dirRange, vec4 colorInt, vec4 cone)
{
    LightData data = calcPointLight(worldPos, posType, dirRange, colorInt);
    if (dot(data.radiance, data.radiance) <= 0.000001)
        return data;

    vec3 spotDir = safeNormalize(dirRange.xyz, vec3(0.0, -1.0, 0.0));
    float cosTheta = dot(data.direction, -spotDir);
    if (cosTheta <= cone.y) {
        data.radiance = vec3(0.0, 0.0, 0.0);
        return data;
    }

    float factor = saturate((cosTheta - cone.y) / max(cone.x - cone.y, 0.0001));
    data.radiance *= factor;
    return data;
}

vec3 calcLightRadiance(LightData data, vec3 V, vec3 N, float NdV, Material mat)
{
    float NdL = saturate(dot(N, data.direction));
    if (NdL <= 0.0)
        return vec3(0.0, 0.0, 0.0);

    return BRDF(V, data.direction, N, NdV, NdL, mat) * data.radiance * NdL;
}
vec3 evaluateLightingFiltered(vec3 worldPos, vec3 N, vec3 V, Material mat, float typeMin, float typeMax)
{
    N = safeNormalize(N, vec3(0.0, 1.0, 0.0));
    mat.a = specularAntiAliasing(N, mat.a);

    vec3 result = vec3(0.0, 0.0, 0.0);
    int count = int(u_lightsCount.x);
    float NdV = abs(dot(N, V)) + 1e-5;

    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        if (i >= count) break;

        vec4 posType  = u_lightsPosType[i];
        vec4 dirRange = u_lightsDirRange[i];
        vec4 colorInt = u_lightsColorInt[i];
        vec4 cone     = u_lightsCone[i];

        float lightType = posType.w;
        if (lightType < typeMin || lightType >= typeMax)
            continue;

        LightData data;
        if (lightType < 0.5) {
            data = calcDirectionalLight(dirRange, colorInt);
        } else if (lightType < 1.5) {
            data = calcPointLight(worldPos, posType, dirRange, colorInt);
        } else {
            data = calcSpotLight(worldPos, posType, dirRange, colorInt, cone);
        }

        if (dot(data.radiance, data.radiance) <= 0.000001)
            continue;

        result += sanitizeColor(calcLightRadiance(data, V, N, NdV, mat), 4096.0);
    }

    return sanitizeColor(result, 4096.0);
}

vec3 evaluateLighting(vec3 worldPos, vec3 N, vec3 V, Material mat)
{
    return evaluateLightingFiltered(worldPos, N, V, mat, -1.0, 3.0);
}

vec2 poissonDisk(int i)
{
    if (i ==  0) return vec2(-0.94201624, -0.39906216);
    if (i ==  1) return vec2( 0.94558609, -0.76890725);
    if (i ==  2) return vec2(-0.09418410, -0.92938870);
    if (i ==  3) return vec2( 0.34495938,  0.29387760);
    if (i ==  4) return vec2(-0.91588581,  0.45771432);
    if (i ==  5) return vec2(-0.81544232, -0.87912464);
    if (i ==  6) return vec2(-0.38277543,  0.27676845);
    if (i ==  7) return vec2( 0.97484398,  0.75648379);
    if (i ==  8) return vec2( 0.44323325, -0.97511554);
    if (i ==  9) return vec2( 0.53742981, -0.47373420);
    if (i == 10) return vec2(-0.26496911, -0.41893023);
    if (i == 11) return vec2( 0.79197514,  0.19090188);
    if (i == 12) return vec2(-0.24188840,  0.99706507);
    if (i == 13) return vec2(-0.81409955,  0.91437590);
    if (i == 14) return vec2( 0.19984126,  0.78641367);
    return vec2( 0.14383161, -0.14100790);
}

float interleavedGradientNoise(vec2 pixel)
{
    return fract(52.9829189 * fract(dot(pixel, vec2(0.06711056, 0.00583715))));
}

int selectCascade(float viewDepth, int count)
{
    if (count > 1 && viewDepth < u_cascadeSplits.x) return 0;
    if (count > 1 && viewDepth < u_cascadeSplits.y) return 1;
    if (count > 2 && viewDepth < u_cascadeSplits.z) return 2;
    if (count > 3 && viewDepth < u_cascadeSplits.w) return 3;
    return count - 1;
}

mat4 cascadeMatrix(int index)
{
    if (index == 0) return u_cascadeViewProj[0];
    if (index == 1) return u_cascadeViewProj[1];
    if (index == 2) return u_cascadeViewProj[2];
    return u_cascadeViewProj[3];
}

vec4 cascadeOffset(int index)
{
    if (index == 0) return u_cascadeOffsets[0];
    if (index == 1) return u_cascadeOffsets[1];
    if (index == 2) return u_cascadeOffsets[2];
    return u_cascadeOffsets[3];
}

vec3 cascadeDebugColor(int index)
{
    if (index == 0) return vec3(1.0, 0.35, 0.35);
    if (index == 1) return vec3(0.35, 1.0, 0.35);
    if (index == 2) return vec3(0.35, 0.55, 1.0);
    return vec3(1.0, 1.0, 0.35);
}

float computeShadow(vec3 worldPos, vec3 normal, float viewDepth, float NdL, vec2 pixelCoord)
{
    int count = int(u_shadowParams.x);
    if (count <= 0)
        return 1.0;

    int cascade = selectCascade(viewDepth, count);
    vec4 offset = cascadeOffset(cascade);

    float slope = clamp(1.0 - NdL, 0.0, 1.0);
    vec3 biased = worldPos + normal * (offset.z * u_shadowParams2.y * (1.0 + 2.0 * slope));

    vec4 coord = mul(cascadeMatrix(cascade), vec4(biased, 1.0));
    vec3 proj = coord.xyz / coord.w;

    vec2 uv = proj.xy * 0.5 + 0.5;
    if (u_shadowParams.w < 0.5)
        uv.y = 1.0 - uv.y;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return 1.0;

    float receiver = proj.z;
#if BGFX_SHADER_LANGUAGE_GLSL
    receiver = receiver * 0.5 + 0.5;
#endif
    if (receiver > 1.0)
        return 1.0;

    float bias = u_shadowParams.y * (1.0 + 2.5 * slope) * offset.w;

    vec2 atlasUV = uv * u_shadowParams2.x + offset.xy;
    float radius = u_shadowParams.z * 1.5;

    float angle = interleavedGradientNoise(pixelCoord) * 6.2831853;
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotation = mat2(c, -s, s, c);

    float shadow = 0.0;
    for (int i = 0; i < 16; ++i)
    {
        vec2 sampleOffset = mul(rotation, poissonDisk(i)) * radius;
        float occluder = texture2DLod(s_shadowMap, atlasUV + sampleOffset, 0.0).r;
        shadow += (receiver - bias > occluder) ? 0.0 : 1.0;
    }
    return shadow / 16.0;
}

float acesTonemapScalar(float x)
{
    x = sanitizeScalar(x, 4096.0);
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

vec3 acesTonemap(vec3 x)
{
    x = sanitizeColor(x, 4096.0);
    const vec3 lum_coeff = vec3(0.2126, 0.7152, 0.0722);
    float luminance = dot(x, lum_coeff);
    if (luminance <= 0.000001)
        return vec3(0.0, 0.0, 0.0);

    float mapped_luminance = acesTonemapScalar(luminance);
    return sanitizeColor(x * (mapped_luminance / luminance), 1.0);
}
vec3 hemisphereAmbient(vec3 N, vec3 skyColor, vec3 groundColor)
{
    float t = N.y * 0.5 + 0.5;
    return mix(groundColor, skyColor, t);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    float oneMinusRough = 1.0 - roughness;
    return F0 + (max(vec3(oneMinusRough, oneMinusRough, oneMinusRough), F0) - F0) *
                pow(saturate(1.0 - cosTheta), 5.0);
}

vec3 envBRDFApprox(vec3 F0, float roughness, float NdV)
{
    vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    vec4 c1 = vec4( 1.0,  0.0425,  1.04, -0.04);
    vec4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NdV)) * r.x + r.y;
    vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
    return sanitizeColor(F0 * AB.x + vec3(AB.y, AB.y, AB.y), 16.0);
}

vec3 dominantSpecularDirection(vec3 N, vec3 R, float roughness)
{
    float blend = roughness * roughness;
    return safeNormalize(mix(R, N, blend), N);
}

vec3 evaluateEnvironmentSpecular(vec3 N, vec3 V, Material mat, vec3 groundColor)
{
    vec3 R = reflect(-V, N);
    vec3 dominant_R = dominantSpecularDirection(N, R, mat.roughness);

    float sky_luma = dot(u_envSkyColor.xyz, vec3(0.2126, 0.7152, 0.0722));
    vec3 sky_env = mix(u_envSkyColor.xyz, vec3(sky_luma, sky_luma, sky_luma), 0.6);
    vec3 ground_env = vec3(sky_luma, sky_luma, sky_luma) * 0.5;
    vec3 prefiltered_env = hemisphereAmbient(dominant_R, sky_env, ground_env);

    vec3 sun_dir = safeNormalize(u_envSunDir.xyz, vec3(0.0, 1.0, 0.0));
    float sun_alignment = saturate(dot(dominant_R, sun_dir));
    float sun_power = mix(2048.0, 8.0, sqrt(saturate(mat.roughness)));
    float sun_lobe = pow(sun_alignment, sun_power) * u_envSunDir.w;
    prefiltered_env += u_envSunColor.xyz * sun_lobe;

    float NdV = saturate(dot(N, V));
    vec3 env_brdf = envBRDFApprox(mat.F0, mat.roughness, NdV);
    return sanitizeColor(prefiltered_env * env_brdf * mat.occlusion, 4096.0);
}

vec2 dirToEquirectUV(vec3 d)
{
    float u = atan2(d.z, d.x) * (INV_PI * 0.5) + 0.5;
    float v = acos(clamp(d.y, -1.0, 1.0)) * INV_PI;
    return vec2(u, v);
}

vec3 sampleEnvLod(vec3 dir, float lod)
{
    return texture2DLod(s_envMap, dirToEquirectUV(dir), lod).rgb * u_iblParams.z;
}

vec3 evaluateIBLSpecular(vec3 N, vec3 V, Material mat)
{
    vec3 R = reflect(-V, N);
    vec3 dominant_R = dominantSpecularDirection(N, R, mat.roughness);
    float lod = saturate(mat.roughness) * u_iblParams.y;
    vec3 prefiltered = sampleEnvLod(dominant_R, lod);

    float NdV = saturate(dot(N, V));
    vec3 env_brdf = envBRDFApprox(mat.F0, mat.roughness, NdV);
    return sanitizeColor(prefiltered * env_brdf * mat.occlusion, 4096.0);
}

vec3 evaluateIBLDiffuse(vec3 N, Material mat)
{
    vec3 irradiance = sampleEnvLod(N, max(u_iblParams.y - 1.0, 0.0));
    return sanitizeColor(irradiance * mat.diffuse * mat.occlusion, 4096.0);
}

vec3 evaluateAmbient(vec3 N, vec3 V, Material mat, vec3 skyColor, vec3 groundColor)
{
    N = safeNormalize(N, vec3(0.0, 1.0, 0.0));
    V = safeNormalize(V, vec3(0.0, 0.0, 1.0));
    
    vec3 diffuse_irradiance = hemisphereAmbient(N, skyColor, groundColor);
    vec3 hemi = diffuse_irradiance * mat.diffuse * mat.occlusion + evaluateEnvironmentSpecular(N, V, mat, groundColor);

    vec3 ibl = evaluateIBLDiffuse(N, mat) + evaluateIBLSpecular(N, V, mat);

    vec3 result = mix(hemi, ibl, step(0.5, u_iblParams.x));
    return sanitizeColor(result, 4096.0);
}

#endif
