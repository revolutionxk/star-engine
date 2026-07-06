$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_src, 0);

uniform vec4 u_blurParams;

void main()
{
    vec2 d = u_blurParams.xy;
    vec3 sum = texture2D(s_src, v_texcoord0).rgb * 0.227027;
    sum += texture2D(s_src, v_texcoord0 + d * 1.384615).rgb * 0.316216;
    sum += texture2D(s_src, v_texcoord0 - d * 1.384615).rgb * 0.316216;
    sum += texture2D(s_src, v_texcoord0 + d * 3.230769).rgb * 0.070270;
    sum += texture2D(s_src, v_texcoord0 - d * 3.230769).rgb * 0.070270;
    gl_FragColor = vec4(sum, 1.0);
}
