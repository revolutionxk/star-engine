$input a_position
$output v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 u_postParams;

void main()
{
    vec2 uv = a_position.xy * 0.5 + 0.5;
    if (u_postParams.x > 0.5)
        uv.y = 1.0 - uv.y;
    v_texcoord0 = uv;
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
}
