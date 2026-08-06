$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_hdr, 0);
SAMPLER2D(s_bloom, 1);
SAMPLER2D(s_ao, 2);

uniform vec4 u_postParams;
uniform vec4 u_tonemapParams;

float sanitizeScalar(float x, float maxValue)
{
    if (x != x)
        return 0.0;
    return clamp(x, 0.0, maxValue);
}

vec3 sanitizeColor(vec3 c, float maxValue)
{
    return vec3(sanitizeScalar(c.x, maxValue), sanitizeScalar(c.y, maxValue), sanitizeScalar(c.z, maxValue));
}

float luminance(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

float acesScalar(float x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

vec3 tonemapAces(vec3 x)
{
    float luma = luminance(x);
    if (luma <= 0.000001)
        return vec3_splat(0.0);
    return sanitizeColor(x * (acesScalar(luma) / luma), 1.0);
}

vec3 tonemapReinhard(vec3 x)
{
    float luma = luminance(x);
    if (luma <= 0.000001)
        return vec3_splat(0.0);
    float mapped = luma / (1.0 + luma);
    return sanitizeColor(x * (mapped / luma), 1.0);
}

vec3 agxDefaultContrast(vec3 x)
{
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return 15.5 * x4 * x2
         - 40.14 * x4 * x
         + 31.96 * x4
         - 6.868 * x2 * x
         + 0.4298 * x2
         + 0.1191 * x
         - 0.00232;
}

vec3 tonemapAgx(vec3 x)
{
    const float minEv = -12.47393;
    const float maxEv = 4.026069;

    vec3 inset = vec3(
        dot(x, vec3(0.842479062253094,  0.0784335999999992, 0.0792237451477643)),
        dot(x, vec3(0.0423282422610123, 0.878468636469772,  0.0791661274605434)),
        dot(x, vec3(0.0423756549057051, 0.0784336,          0.879142973793104)));

    inset = max(inset, vec3_splat(1e-10));
    inset = log2(inset);
    inset = (inset - minEv) / (maxEv - minEv);
    inset = saturate(inset);

    vec3 mapped = agxDefaultContrast(inset);

    vec3 outset = vec3(
        dot(mapped, vec3( 1.19687900512017,  -0.0980208811401368, -0.0990297440797205)),
        dot(mapped, vec3(-0.0528968517574562, 1.15190312990417,   -0.0989611768448433)),
        dot(mapped, vec3(-0.0529716355144438,-0.0980434501171241,  1.15107367264116)));

    return sanitizeColor(saturate(outset), 1.0);
}

float gt7Scalar(float v)
{
    const float maxDisplay = 1.0;
    const float contrast = 1.0;
    const float toeStart = 0.22;
    const float midLength = 0.4;
    const float blackTightness = 1.33;

    float l0 = (maxDisplay - toeStart) * midLength / contrast;
    float s0 = toeStart + l0;
    float s1 = toeStart + contrast * l0;
    float c2 = contrast * maxDisplay / max(maxDisplay - s1, 1e-4);

    float toe = toeStart * pow(max(v / toeStart, 1e-6), blackTightness);
    float mid = toeStart + contrast * (v - toeStart);
    float shoulder = maxDisplay - (maxDisplay - s1) * exp(-c2 * (v - s0) / maxDisplay);

    if (v < toeStart) return toe;
    if (v < s0) return mid;
    return shoulder;
}

vec3 tonemapGt7(vec3 x)
{
    return sanitizeColor(saturate(vec3(gt7Scalar(x.x), gt7Scalar(x.y), gt7Scalar(x.z))), 1.0);
}

vec3 applyTonemap(vec3 x, int mode)
{
    if (mode == 1) return tonemapAgx(x);
    if (mode == 2) return tonemapReinhard(x);
    if (mode == 3) return tonemapGt7(x);
    if (mode == 4) return sanitizeColor(saturate(x), 1.0);
    return tonemapAces(x);
}

void main()
{
    vec3 hdr = texture2D(s_hdr, v_texcoord0).rgb;

    float ao = texture2D(s_ao, v_texcoord0).r;
    hdr *= mix(1.0, ao, u_postParams.w);

    vec3 bloom = texture2D(s_bloom, v_texcoord0).rgb;
    hdr += bloom * u_postParams.z;

    hdr = sanitizeColor(hdr * u_postParams.y, 4096.0);

    vec3 color = applyTonemap(hdr, int(u_tonemapParams.x));
    color = pow(max(color, vec3_splat(0.0)), vec3_splat(1.0 / 2.2));
    gl_FragColor = vec4(color, 1.0);
}
