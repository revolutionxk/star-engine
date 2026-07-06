$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_hdr, 0);
SAMPLER2D(s_bloom, 1);
SAMPLER2D(s_ao, 2);

uniform vec4 u_postParams;

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

void main()
{
    vec3 hdr = texture2D(s_hdr, v_texcoord0).rgb;

    float ao = texture2D(s_ao, v_texcoord0).r;
    hdr *= mix(1.0, ao, u_postParams.w);

    vec3 bloom = texture2D(s_bloom, v_texcoord0).rgb;
    hdr += bloom * u_postParams.z;

    vec3 color = acesTonemap(hdr * u_postParams.y);
    color = pow(max(color, vec3_splat(0.0)), vec3_splat(1.0 / 2.2));
    gl_FragColor = vec4(color, 1.0);
}
