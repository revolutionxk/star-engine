$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_lum, 0);
SAMPLER2D(s_prev, 1);

uniform vec4 u_exposureParams;

void main()
{
    float sum = 0.0;
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            vec2 uv = (vec2(float(x), float(y)) + 0.5) * 0.25;
            sum += texture2D(s_lum, uv).r;
        }
    }

    float avgLuma = exp2(sum * 0.0625);
    float ev = log2(max(avgLuma, 1e-6) * 8.0);
    ev = clamp(ev, u_exposureParams.x, u_exposureParams.y);

    float prev = texture2D(s_prev, vec2(0.5, 0.5)).r;
    float blended = u_exposureParams.w > 0.5 ? ev : mix(prev, ev, u_exposureParams.z);

    gl_FragColor = vec4(blended, 1.0 / (1.2 * exp2(blended)), 0.0, 1.0);
}
