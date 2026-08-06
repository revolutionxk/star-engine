$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_src, 0);

uniform vec4 u_bloomParams;
uniform vec4 u_bloomTexel;

vec3 fetch(vec2 uv)
{
    return texture2D(s_src, uv).rgb;
}

vec3 prefilter(vec3 c)
{
    if (u_bloomParams.z < 0.5)
        return c;

    float luma = dot(c, vec3(0.2126, 0.7152, 0.0722));
    float threshold = u_bloomParams.x;
    float knee = max(u_bloomParams.y, 0.0001);
    float soft = clamp((luma - threshold + knee) / (2.0 * knee), 0.0, 1.0);
    float contribution = max(soft * soft, step(threshold, luma)) * max(luma - threshold, 0.0) / max(luma, 0.0001);
    return c * contribution;
}

void main()
{
    vec2 t = u_bloomTexel.xy;
    vec2 uv = v_texcoord0;

    vec3 a = fetch(uv + vec2(-2.0, 2.0) * t);
    vec3 b = fetch(uv + vec2( 0.0, 2.0) * t);
    vec3 c = fetch(uv + vec2( 2.0, 2.0) * t);
    vec3 d = fetch(uv + vec2(-2.0, 0.0) * t);
    vec3 e = fetch(uv);
    vec3 f = fetch(uv + vec2( 2.0, 0.0) * t);
    vec3 g = fetch(uv + vec2(-2.0,-2.0) * t);
    vec3 h = fetch(uv + vec2( 0.0,-2.0) * t);
    vec3 i = fetch(uv + vec2( 2.0,-2.0) * t);

    vec3 j = fetch(uv + vec2(-1.0, 1.0) * t);
    vec3 k = fetch(uv + vec2( 1.0, 1.0) * t);
    vec3 l = fetch(uv + vec2(-1.0,-1.0) * t);
    vec3 m = fetch(uv + vec2( 1.0,-1.0) * t);

    vec3 sum = e * 0.125;
    sum += (a + c + g + i) * 0.03125;
    sum += (b + d + f + h) * 0.0625;
    sum += (j + k + l + m) * 0.125;

    gl_FragColor = vec4(prefilter(sum), 1.0);
}
