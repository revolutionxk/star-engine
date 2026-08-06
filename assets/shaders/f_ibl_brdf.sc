$input v_texcoord0

#include <bgfx_shader.sh>
#include "ibl.sh"

#define BRDF_SAMPLES 128

void main()
{
    float ndv = max(v_texcoord0.x, 0.002);
    float roughness = max(v_texcoord0.y, 0.002);

    vec3 n = vec3(0.0, 1.0, 0.0);
    vec3 v = vec3(sqrt(1.0 - ndv * ndv), ndv, 0.0);

    float scale = 0.0;
    float offset = 0.0;

    float invCount = 1.0 / float(BRDF_SAMPLES);
    for (int i = 0; i < BRDF_SAMPLES; ++i)
    {
        vec2 xi = iblHammersley(float(i), invCount);
        vec3 h = iblImportanceGGX(xi, roughness, n);
        vec3 l = 2.0 * dot(v, h) * h - v;

        float ndl = dot(n, l);
        if (ndl > 0.0)
        {
            float ndh = max(dot(n, h), 0.0);
            float vdh = max(dot(v, h), 0.0);

            float g = iblGeometrySmith(ndv, ndl, roughness);
            float gVis = g * vdh / max(ndh * ndv, 1e-5);
            float fc = pow(1.0 - vdh, 5.0);

            scale += (1.0 - fc) * gVis;
            offset += fc * gVis;
        }
    }

    gl_FragColor = vec4(scale * invCount, offset * invCount, 0.0, 1.0);
}
