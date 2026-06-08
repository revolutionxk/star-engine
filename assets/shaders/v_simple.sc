$input a_position, a_normal, a_color0, a_texcoord0
$output v_color0, v_normal, v_position, v_texcoord0

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "shaderlib.sh"

uniform vec4 u_camPos;

vec3 transformNormal(mat4 model, vec3 normal)
{
    return normalize(mul(cofactor(model), normal));
}

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    vec3 normal = transformNormal(u_model[0], a_normal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    float ndotl = max(dot(normal, lightDir), 0.2);

    vec4 baseColor = (a_color0.r + a_color0.g + a_color0.b > 0.001) ? a_color0 : vec4(1.0, 1.0, 1.0, 1.0);

    v_color0 = vec4(ndotl, ndotl, ndotl, 1.0) * baseColor;
    v_normal = normal;
    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_texcoord0 = a_texcoord0;
}