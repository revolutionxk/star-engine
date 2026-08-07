$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_src, 0);

uniform vec4 u_lumParams;

void main()
{
    vec2 spacing = u_lumParams.yz;
    bool convert = u_lumParams.x > 0.5;

    float sum = 0.0;
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            vec2 offset = (vec2(float(x), float(y)) - 1.5) * spacing;
            vec3 c = texture2D(s_src, v_texcoord0 + offset).rgb;
            float luma = dot(c, vec3(0.2126, 0.7152, 0.0722));
            sum += convert ? log2(max(luma, 1e-4)) : c.r;
        }
    }

    gl_FragColor = vec4(sum * 0.0625, 0.0, 0.0, 1.0);
}
