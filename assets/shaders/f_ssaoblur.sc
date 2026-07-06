$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_ao, 0);
SAMPLER2D(s_depth, 1);

uniform vec4 u_ssaoBlurParams;

void main()
{
    vec2 texel = u_ssaoBlurParams.xy;
    float sharp = u_ssaoBlurParams.z;

    float centerDepth = texture2D(s_depth, v_texcoord0).x;
    float sum = 0.0;
    float wsum = 0.0;

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            vec2 o = vec2(float(x), float(y)) * texel;
            float ao = texture2D(s_ao, v_texcoord0 + o).x;
            float d = texture2D(s_depth, v_texcoord0 + o).x;
            float w = exp(-abs(d - centerDepth) * sharp);
            sum += ao * w;
            wsum += w;
        }
    }

    float result = sum / max(wsum, 0.0001);
    gl_FragColor = vec4(result, result, result, 1.0);
}
