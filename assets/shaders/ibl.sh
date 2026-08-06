#ifndef STAR_IBL_SH
#define STAR_IBL_SH

#define IBL_PI      3.14159265359
#define IBL_TWO_PI  6.28318530718
#define IBL_INV_PI  0.31830988618

vec2 dirToEquirectUV(vec3 d)
{
    float u = atan2(d.z, d.x) * (IBL_INV_PI * 0.5) + 0.5;
    float v = acos(clamp(d.y, -1.0, 1.0)) * IBL_INV_PI;
    return vec2(u, v);
}

vec3 equirectUVToDir(vec2 uv)
{
    float phi = (uv.x - 0.5) * IBL_TWO_PI;
    float theta = uv.y * IBL_PI;
    float sinTheta = sin(theta);
    return vec3(sinTheta * cos(phi), cos(theta), sinTheta * sin(phi));
}

vec2 iblHammersley(float i, float invCount)
{
    return vec2(i * invCount, fract(i * 0.6180339887498949));
}

void iblBasis(vec3 n, out vec3 tx, out vec3 ty)
{
    vec3 up = abs(n.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    tx = normalize(cross(up, n));
    ty = cross(n, tx);
}

vec3 iblImportanceGGX(vec2 xi, float roughness, vec3 n)
{
    float a = roughness * roughness;
    float phi = IBL_TWO_PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(max(1.0 - cosTheta * cosTheta, 0.0));

    vec3 tx, ty;
    iblBasis(n, tx, ty);

    vec3 dir = tx * (sinTheta * cos(phi)) + ty * (sinTheta * sin(phi)) + n * cosTheta;
    return normalize(dir);
}

float iblGeometrySmith(float ndv, float ndl, float roughness)
{
    float k = roughness * roughness * 0.5;
    float gv = ndv / (ndv * (1.0 - k) + k);
    float gl = ndl / (ndl * (1.0 - k) + k);
    return gv * gl;
}

#endif
