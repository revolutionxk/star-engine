$input v_shadowCoord

#include <bgfx_shader.sh>

void main()
{
    float depth = v_shadowCoord.z / v_shadowCoord.w;
#if BGFX_SHADER_LANGUAGE_GLSL
    depth = depth * 0.5 + 0.5;
#endif
    gl_FragColor = vec4(depth, depth, depth, 1.0);
}
