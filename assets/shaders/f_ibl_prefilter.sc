$input v_texcoord0

#include <bgfx_shader.sh>
#include "ibl.sh"

SAMPLER2D(s_src, 0);

uniform vec4 u_iblBake;

#define PREFILTER_SAMPLES 64

void main()
{
    float roughness = u_iblBake.x;
    vec3 n = equirectUVToDir(v_texcoord0);

    if (roughness < 0.01)
    {
        gl_FragColor = vec4(texture2DLod(s_src, v_texcoord0, 0.0).rgb, 1.0);
        return;
    }

    float saTexel = u_iblBake.y;
    float maxLod = u_iblBake.z;
    float invCount = 1.0 / float(PREFILTER_SAMPLES);

    float a = roughness * roughness;
    float a2 = a * a;

    vec3 sum = vec3_splat(0.0);
    float weight = 0.0;

    for (int i = 0; i < PREFILTER_SAMPLES; ++i)
    {
        vec2 xi = iblHammersley(float(i), invCount);
        vec3 h = iblImportanceGGX(xi, roughness, n);
        vec3 l = 2.0 * dot(n, h) * h - n;

        float ndl = dot(n, l);
        if (ndl > 0.0)
        {
            float ndh = max(dot(n, h), 0.0);
            float denom = ndh * ndh * (a2 - 1.0) + 1.0;
            float d = a2 / max(IBL_PI * denom * denom, 1e-7);
            float pdf = d * 0.25 + 1e-4;

            float saSample = 1.0 / (float(PREFILTER_SAMPLES) * pdf);
            float lod = clamp(0.5 * log2(saSample / saTexel), 0.0, maxLod);

            sum += texture2DLod(s_src, dirToEquirectUV(l), lod).rgb * ndl;
            weight += ndl;
        }
    }

    gl_FragColor = vec4(sum / max(weight, 1e-4), 1.0);
}
