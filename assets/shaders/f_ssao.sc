$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_depth, 0);

uniform mat4 u_ssaoInvProj;
uniform mat4 u_ssaoProj;

uniform vec4 u_ssaoParams;
uniform vec4 u_ssaoTexel;

#define GTAO_PI 3.14159265
#define GTAO_HALFPI 1.57079632
#define GTAO_SLICES 3
#define GTAO_STEPS 4

float gtaoIGN(vec2 pix)
{
    return fract(52.9829189 * fract(dot(pix, vec2(0.06711056, 0.00583715))));
}

vec3 gtaoViewPos(vec2 uv, float d)
{
    vec4 clip = vec4(uv * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
    vec4 view = mul(u_ssaoInvProj, clip);
    return view.xyz / view.w;
}

float gtaoIntegrateArc(float h1, float h2, float n)
{
    float cosN = cos(n);
    float sinN = sin(n);
    return 0.25 * ((-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN) +
                   (-cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN));
}

void main()
{
    vec2 uv = v_texcoord0;
    vec2 texel = u_ssaoTexel.xy;
    float depth = texture2D(s_depth, uv).x;
    vec3 P = gtaoViewPos(uv, depth);
    vec3 V = normalize(-P);

    vec3 N = normalize(cross(dFdx(P), dFdy(P)));
    if (dot(N, V) < 0.0)
        N = -N;

    if (depth >= 0.9999) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float radius = u_ssaoParams.x;

    vec2 radiusUV = vec2(radius * 0.5 * u_ssaoProj[0].x, radius * 0.5 * u_ssaoProj[1].y) / max(-P.z, 0.001);
    radiusUV = clamp(radiusUV, texel * float(GTAO_STEPS), vec2(0.15, 0.15));

    vec2 pix = uv / max(texel, vec2(1e-6, 1e-6));
    float rot = gtaoIGN(pix);
    float jit = gtaoIGN(pix + 71.3);

    float visibility = 0.0;
    float weightSum = 0.0;
    for (int slice = 0; slice < GTAO_SLICES; ++slice)
    {
        float phi = (float(slice) + rot) * GTAO_PI / float(GTAO_SLICES);
        vec2 dir2D = vec2(cos(phi), sin(phi));

        vec2 nUV = uv + dir2D * texel;
        vec3 Pn = gtaoViewPos(nUV, texture2DLod(s_depth, nUV, 0.0).x);
        vec3 sliceDir = Pn - P;
        sliceDir = sliceDir - V * dot(sliceDir, V);
        float sdLen = length(sliceDir);
        if (sdLen < 1e-5)
            continue;
        sliceDir /= sdLen;

        vec3 planeN = normalize(cross(V, sliceDir));
        vec3 projN = N - planeN * dot(N, planeN);
        float projNLen = length(projN);
        if (projNLen < 1e-4)
            continue;
        vec3 projNn = projN / projNLen;

        float n = acos(clamp(dot(projNn, V), -1.0, 1.0)) * sign(dot(projNn, sliceDir));

        float cHpos = -1.0;
        float cHneg = -1.0;
        for (int st = 1; st <= GTAO_STEPS; ++st)
        {
            float t = (float(st) - jit) / float(GTAO_STEPS);
            vec2 offs = dir2D * radiusUV * t;

            vec2 up = uv + offs;
            if (up.x > 0.0 && up.x < 1.0 && up.y > 0.0 && up.y < 1.0) {
                vec3 dvp = gtaoViewPos(up, texture2DLod(s_depth, up, 0.0).x) - P;
                float lenp = length(dvp);
                if (lenp > 1e-3) {
                    float fallp = clamp(1.0 - lenp / radius, 0.0, 1.0);
                    cHpos = max(cHpos, (dot(dvp, V) / lenp) * fallp);
                }
            }

            vec2 un = uv - offs;
            if (un.x > 0.0 && un.x < 1.0 && un.y > 0.0 && un.y < 1.0) {
                vec3 dvn = gtaoViewPos(un, texture2DLod(s_depth, un, 0.0).x) - P;
                float lenn = length(dvn);
                if (lenn > 1e-3) {
                    float falln = clamp(1.0 - lenn / radius, 0.0, 1.0);
                    cHneg = max(cHneg, (dot(dvn, V) / lenn) * falln);
                }
            }
        }

        float h1 = -acos(clamp(cHneg, -1.0, 1.0));
        float h2 = acos(clamp(cHpos, -1.0, 1.0));
        h1 = n + max(h1 - n, -GTAO_HALFPI);
        h2 = n + min(h2 - n, GTAO_HALFPI);

        visibility += projNLen * gtaoIntegrateArc(h1, h2, n);
        weightSum += projNLen;
    }

    visibility /= max(weightSum, 1e-4);
    visibility = pow(clamp(visibility, 0.0, 1.0), max(u_ssaoParams.y, 0.01));

    float fadeStart = max(u_ssaoParams.z, 1.0);
    float distFade = 1.0 - clamp((-P.z - fadeStart) / fadeStart, 0.0, 1.0);
    visibility = mix(1.0, visibility, distFade);

    float grazeFade = smoothstep(0.08, 0.25, NdotV);
    visibility = mix(1.0, visibility, grazeFade);

    gl_FragColor = vec4(visibility, visibility, visibility, 1.0);
}
