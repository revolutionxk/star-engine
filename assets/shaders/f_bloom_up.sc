$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_src, 0);
SAMPLER2D(s_prev, 1);

uniform vec4 u_bloomTexel;

void main()
{
    vec2 t = u_bloomTexel.xy * u_bloomTexel.z;
    vec2 uv = v_texcoord0;

    vec3 sum = texture2D(s_src, uv + vec2(-1.0,  1.0) * t).rgb * 1.0;
    sum += texture2D(s_src, uv + vec2( 0.0,  1.0) * t).rgb * 2.0;
    sum += texture2D(s_src, uv + vec2( 1.0,  1.0) * t).rgb * 1.0;
    sum += texture2D(s_src, uv + vec2(-1.0,  0.0) * t).rgb * 2.0;
    sum += texture2D(s_src, uv).rgb * 4.0;
    sum += texture2D(s_src, uv + vec2( 1.0,  0.0) * t).rgb * 2.0;
    sum += texture2D(s_src, uv + vec2(-1.0, -1.0) * t).rgb * 1.0;
    sum += texture2D(s_src, uv + vec2( 0.0, -1.0) * t).rgb * 2.0;
    sum += texture2D(s_src, uv + vec2( 1.0, -1.0) * t).rgb * 1.0;
    sum *= 1.0 / 16.0;

    gl_FragColor = vec4(sum + texture2D(s_prev, uv).rgb, 1.0);
}
