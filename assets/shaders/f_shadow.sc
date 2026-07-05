$input v_shadowCoord

#include <bgfx_shader.sh>

void main()
{
    float depth = (v_shadowCoord.z / v_shadowCoord.w) * 0.5 + 0.5;
    gl_FragColor = vec4(depth, depth, depth, 1.0);
}
