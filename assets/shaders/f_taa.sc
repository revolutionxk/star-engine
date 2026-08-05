$input v_texcoord0

#include <bgfx_shader.sh>
#include "shaderlib.sh"

SAMPLER2D(s_current, 0);
SAMPLER2D(s_history, 1);
SAMPLER2D(s_depth, 2);
SAMPLER2D(s_velocity, 3);

uniform mat4 u_taaCurInvVP;
uniform mat4 u_taaPrevVP;
uniform vec4 u_taaParams;
uniform vec4 u_taaTexel;

void main()
{
    float flip = u_taaParams.x;
    vec2 uvTex = v_texcoord0;
    vec2 uvRaw = uvTex;
    if (flip > 0.5)
        uvRaw.y = 1.0 - uvRaw.y;

    vec3 cur = texture2DLod(s_current, uvTex, 0.0).rgb;

    float depth = texture2DLod(s_depth, uvTex, 0.0).x;
    vec2 velocity = texture2DLod(s_velocity, uvTex, 0.0).xy;
    vec2 prevRawMV = uvRaw - velocity;

    vec4 clip = vec4(uvRaw * 2.0 - 1.0, toClipSpaceDepth(depth), 1.0);
    vec4 world = mul(u_taaCurInvVP, clip);
    world /= world.w;
    vec4 pc = mul(u_taaPrevVP, world);
    vec2 prevRawCam = (pc.xy / pc.w) * 0.5 + 0.5;

    vec2 prevRaw = (depth >= 0.9999) ? prevRawCam : prevRawMV;
    vec2 prevTex = prevRaw;
    if (flip > 0.5)
        prevTex.y = 1.0 - prevTex.y;

    vec3 nmin = cur;
    vec3 nmax = cur;
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y) {
            vec3 c = texture2DLod(s_current, uvTex + vec2(float(x), float(y)) * u_taaTexel.xy, 0.0).rgb;
            nmin = min(nmin, c);
            nmax = max(nmax, c);
        }

    vec3 histSample = texture2DLod(s_history, prevTex, 0.0).rgb;
    vec3 hist = clamp(histSample, nmin, nmax);

    float onScreen = (prevRaw.x > 0.0 && prevRaw.x < 1.0 && prevRaw.y > 0.0 && prevRaw.y < 1.0) ? 1.0 : 0.0;
    float blend = u_taaParams.y * u_taaParams.z * onScreen;

    gl_FragColor = vec4(mix(cur, hist, blend), 1.0);
}
