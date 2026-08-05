$input v_screenPos, v_viewDir

#include <bgfx_shader.sh>

uniform vec4 u_sunDirection;
uniform vec4 u_skyLuminanceXYZ;
uniform vec4 u_parameters;
uniform vec4 u_sunLuminance;
uniform vec4 u_perezCoeff[5];

uniform vec4 u_envSkyMode;

SAMPLER2D(s_envMap, 0);

vec2 dirToEquirectUV(vec3 d)
{
    float u = atan2(d.z, d.x) * 0.15915494 + 0.5;
    float v = acos(clamp(d.y, -1.0, 1.0)) * 0.31830988;
    return vec2(u, v);
}

vec3 xyzToRgb(vec3 xyz)
{
    return vec3(
         3.240479 * xyz.x - 1.537150 * xyz.y - 0.498535 * xyz.z,
        -0.969256 * xyz.x + 1.875991 * xyz.y + 0.041556 * xyz.z,
         0.055648 * xyz.x - 0.204043 * xyz.y + 1.057311 * xyz.z
    );
}

vec3 perez(vec3 A, vec3 B, vec3 C, vec3 D, vec3 E, float costeta, float cosgamma)
{
    float _1_costeta = 1.0 / costeta;
    float cos2gamma  = cosgamma * cosgamma;
    float gamma      = acos(cosgamma);
    return (vec3_splat(1.0) + A * exp(B * _1_costeta))
         * (vec3_splat(1.0) + C * exp(D * gamma) + E * cos2gamma);
}

float nrand(vec2 n)
{
    return fract(sin(dot(n.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float n4rand_ss(vec2 n)
{
    float nrnd0 = nrand(n + 0.07 * fract(u_parameters.w));
    float nrnd1 = nrand(n + 0.11 * fract(u_parameters.w + 0.573953));
    return 0.23 * sqrt(max(-log(clamp(nrnd0 + 0.00001, 0.00001, 1.0)), 0.0)) * cos(6.283185 * nrnd1) + 0.5;
}

void main()
{
    vec3 viewDir  = normalize(v_viewDir);
    vec3 lightDir = normalize(u_sunDirection.xyz);
    vec3 skyDir   = vec3(0.0, 1.0, 0.0);

    vec3 A = u_perezCoeff[0].xyz;
    vec3 B = u_perezCoeff[1].xyz;
    vec3 C = u_perezCoeff[2].xyz;
    vec3 D = u_perezCoeff[3].xyz;
    vec3 E = u_perezCoeff[4].xyz;

    float costeta   = max(dot(viewDir, skyDir), 0.001);
    float cosgamma  = clamp(dot(viewDir, lightDir), -0.9999, 0.9999);
    float cosgammas = clamp(dot(skyDir, lightDir), -0.9999, 0.9999);

    vec3 P  = perez(A, B, C, D, E, costeta,  cosgamma);
    vec3 P0 = perez(A, B, C, D, E, 1.0,      cosgammas);

    float rSum = 1.0 / max(u_skyLuminanceXYZ.x + u_skyLuminanceXYZ.y + u_skyLuminanceXYZ.z, 0.001);
    vec3 skyColorxyY = vec3(
        u_skyLuminanceXYZ.x * rSum,
        u_skyLuminanceXYZ.y * rSum,
        u_skyLuminanceXYZ.y
    );

    vec3 Yp  = skyColorxyY * P / max(P0, vec3_splat(0.001));
    float rY = 1.0 / max(Yp.y, 0.001);
    vec3 skyColorXYZ = vec3(
        Yp.x * Yp.z * rY,
        Yp.z,
        (1.0 - Yp.x - Yp.y) * Yp.z * rY
    );

    vec3 skyColor = max(xyzToRgb(skyColorXYZ * u_parameters.z), vec3_splat(0.0));

    float size2 = u_parameters.x * u_parameters.x;
    float dist  = 2.0 * (1.0 - dot(viewDir, lightDir));
    float sun   = exp(-dist / u_parameters.y / size2) + step(dist, size2);
    float sun2  = min(sun * sun, 1.0);

    vec3 color = max(skyColor + sun2 * u_sunLuminance.xyz, vec3_splat(0.0));

    float r = n4rand_ss(v_screenPos);
    color += vec3_splat(r) / 40.0;
    
    vec4 raS = mul(u_invViewProj, vec4(v_screenPos, -1.0, 1.0));
    vec4 raE = mul(u_invViewProj, vec4(v_screenPos, 1.0, 1.0));
    vec3 trueDir = normalize(raE.xyz / raE.w - raS.xyz / raS.w);
    vec3 envColor = texture2D(s_envMap, dirToEquirectUV(trueDir)).rgb * u_envSkyMode.y;
    color = mix(color, envColor, step(0.5, u_envSkyMode.x));

    gl_FragColor = vec4(color, 1.0);
}