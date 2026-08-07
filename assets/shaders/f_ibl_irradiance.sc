$input v_texcoord0

#include <bgfx_shader.sh>
#include "ibl.sh"

SAMPLER2D(s_src, 0);

uniform vec4 u_iblBake;

#define IRRADIANCE_SAMPLES 512

void main()
{
    vec3 n = equirectUVToDir(v_texcoord0);

    vec3 tx, ty;
    iblBasis(n, tx, ty);

    float lod = u_iblBake.x;
    float invCount = 1.0 / float(IRRADIANCE_SAMPLES);

    vec3 sum = vec3_splat(0.0);
    for (int i = 0; i < IRRADIANCE_SAMPLES; ++i)
    {
        vec2 xi = iblHammersley(float(i), invCount);

        float phi = IBL_TWO_PI * xi.x;
        float cosTheta = sqrt(1.0 - xi.y);
        float sinTheta = sqrt(xi.y);

        vec3 dir = tx * (sinTheta * cos(phi)) + ty * (sinTheta * sin(phi)) + n * cosTheta;
        sum += texture2DLod(s_src, dirToEquirectUV(dir), lod).rgb;
    }

    gl_FragColor = vec4(sum * invCount, 1.0);
}
