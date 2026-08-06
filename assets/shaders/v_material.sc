$input a_position, a_normal, a_tangent, a_texcoord0
$output v_position, v_normal, v_tangent, v_texcoord0, v_viewDir, v_curClip, v_prevClip

#include <bgfx_shader.sh>
#include "shaderlib.sh"

uniform vec4 u_camPos;
uniform mat4 u_curViewProjNJ;
uniform mat4 u_prevViewProj;
uniform mat4 u_prevModel;

vec3 transformNormal(mat4 model, vec3 normal)
{
    return normalize(mul(cofactor(model), normal));
}

vec3 transformDirection(mat4 model, vec3 direction)
{
    return normalize(mul(model, vec4(direction, 0.0)).xyz);
}

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_normal = transformNormal(u_model[0], a_normal);
    v_tangent = transformDirection(u_model[0], a_tangent.xyz);
    v_tangent = normalize(v_tangent - v_normal * dot(v_normal, v_tangent));
    v_texcoord0 = a_texcoord0;
    v_viewDir = u_camPos.xyz - v_position;

    vec4 curWorld = mul(u_model[0], vec4(a_position, 1.0));
    vec4 prevWorld = mul(u_prevModel, vec4(a_position, 1.0));
    v_curClip = mul(u_curViewProjNJ, curWorld);
    v_prevClip = mul(u_prevViewProj, prevWorld);
}
