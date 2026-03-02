$input a_position
$output v_worldDir

#include <bgfx_shader.sh>

void main()
{
    gl_Position = vec4(a_position.xy, 0.9999, 1.0);

    // Reconstruct world-space ray direction from NDC
    vec4 world = mul(u_invViewProj, vec4(a_position.xy, 1.0, 1.0));
    v_worldDir = world.xyz / world.w;
}
