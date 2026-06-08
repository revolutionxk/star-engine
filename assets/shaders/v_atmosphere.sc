$input a_position
$output v_screenPos, v_viewDir

#include <bgfx_shader.sh>

void main()
{
    v_screenPos = a_position.xy;

    vec4 rayStart = mul(u_invViewProj, vec4(a_position.xy, -1.0, 1.0));
    vec4 rayEnd   = mul(u_invViewProj, vec4(a_position.xy,  1.0, 1.0));
    rayStart /= rayStart.w;
    rayEnd   /= rayEnd.w;

    v_viewDir   = normalize(rayEnd.xyz - rayStart.xyz);
    v_viewDir.y = abs(v_viewDir.y);

    gl_Position = vec4(a_position.xy, 1.0, 1.0);
}
