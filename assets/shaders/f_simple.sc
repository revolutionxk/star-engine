$input v_color0, v_normal, v_position, v_texcoord0

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "shaderlib.sh"
#include "material.sc"

uniform vec4 u_color;

void main()
{
    vec4 color = v_color0;
    gl_FragColor = color;
}