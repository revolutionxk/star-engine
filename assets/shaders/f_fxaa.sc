$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_src, 0);

uniform vec4 u_fxaaParams;

float fxaaLuma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 texel = u_fxaaParams.xy;

    vec3 rgbM = texture2D(s_src, v_texcoord0).rgb;
    float lumaM = fxaaLuma(rgbM);
    float lumaN = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(0.0, -texel.y)).rgb);
    float lumaS = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(0.0, texel.y)).rgb);
    float lumaW = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(-texel.x, 0.0)).rgb);
    float lumaE = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(texel.x, 0.0)).rgb);

    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaW, lumaE)));
    float lumaMax = max(lumaM, max(max(lumaN, lumaS), max(lumaW, lumaE)));
    float range = lumaMax - lumaMin;

    if (range < max(0.0312, lumaMax * 0.125)) {
        gl_FragColor = vec4(rgbM, 1.0);
        return;
    }

    float lumaNW = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(-texel.x, -texel.y)).rgb);
    float lumaNE = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(texel.x, -texel.y)).rgb);
    float lumaSW = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(-texel.x, texel.y)).rgb);
    float lumaSE = fxaaLuma(texture2D(s_src, v_texcoord0 + vec2(texel.x, texel.y)).rgb);

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * 0.03125, 0.0078125);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -8.0, 8.0) * texel;

    vec3 rgbA = 0.5 * (texture2D(s_src, v_texcoord0 + dir * (1.0 / 3.0 - 0.5)).rgb +
                       texture2D(s_src, v_texcoord0 + dir * (2.0 / 3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture2D(s_src, v_texcoord0 + dir * -0.5).rgb +
                                     texture2D(s_src, v_texcoord0 + dir * 0.5).rgb);

    float lumaB = fxaaLuma(rgbB);
    if (lumaB < lumaMin || lumaB > lumaMax)
        gl_FragColor = vec4(rgbA, 1.0);
    else
        gl_FragColor = vec4(rgbB, 1.0);
}
