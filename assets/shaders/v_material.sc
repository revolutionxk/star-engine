$input a_position, a_normal, a_tangent, a_texcoord0
$output v_position, v_normal, v_tangent, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>

uniform vec4 u_camPos;

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_normal = normalize(mul(u_model[0], vec4(a_normal, 0.0)).xyz);
    v_tangent = normalize(mul(u_model[0], vec4(a_tangent.xyz, 0.0)).xyz);
    v_texcoord0 = a_texcoord0;
    v_viewDir = normalize(u_camPos.xyz - v_position);
}
