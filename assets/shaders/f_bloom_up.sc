$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_src, 0);

uniform vec4 u_bloomTexel;

vec3 fetch(vec2 uv)
{
    vec2 halfTexel = u_bloomTexel.xy * 0.5;
    return texture2D(s_src, clamp(uv, halfTexel, vec2(1.0, 1.0) - halfTexel)).rgb;
}

void main()
{
    vec2 t = u_bloomTexel.xy * u_bloomTexel.z;
    vec2 uv = v_texcoord0;

    vec3 sum = fetch(uv + vec2(-1.0,  1.0) * t) * 1.0;
    sum += fetch(uv + vec2( 0.0,  1.0) * t) * 2.0;
    sum += fetch(uv + vec2( 1.0,  1.0) * t) * 1.0;
    sum += fetch(uv + vec2(-1.0,  0.0) * t) * 2.0;
    sum += fetch(uv) * 4.0;
    sum += fetch(uv + vec2( 1.0,  0.0) * t) * 2.0;
    sum += fetch(uv + vec2(-1.0, -1.0) * t) * 1.0;
    sum += fetch(uv + vec2( 0.0, -1.0) * t) * 2.0;
    sum += fetch(uv + vec2( 1.0, -1.0) * t) * 1.0;
    sum *= 1.0 / 16.0;

    gl_FragColor = vec4(sum, 1.0);
}
