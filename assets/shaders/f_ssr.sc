$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_hdr, 0);
SAMPLER2D(s_depth, 1);
SAMPLER2D(s_scene, 2);
SAMPLER2D(s_gbuffer, 3);

uniform mat4 u_ssrProj;
uniform mat4 u_ssrInvProj;
uniform vec4 u_ssrParams;
uniform vec4 u_ssrTexel;

#define SSR_MAX_STEPS 80

vec3 ssrViewPos(vec2 uvRaw, float d)
{
    vec4 clip = vec4(uvRaw * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
    vec4 view = mul(u_ssrInvProj, clip);
    return view.xyz / view.w;
}

vec3 ssrPosAt(vec2 uvTex, float flip)
{
    vec2 uvRaw = uvTex;
    if (flip > 0.5)
        uvRaw.y = 1.0 - uvRaw.y;
    float d = texture2DLod(s_depth, uvTex, 0.0).x;
    return ssrViewPos(uvRaw, d);
}

void main()
{
    float flip = u_ssrTexel.z;
    vec2 uvTex = v_texcoord0;
    vec2 texel = u_ssrTexel.xy;

    if (u_ssrTexel.w > 0.5) {
        float roughness = texture2DLod(s_gbuffer, uvTex, 0.0).a;
        float offsScale = mix(0.75, 6.0, roughness);

        float centerDepth = texture2DLod(s_depth, uvTex, 0.0).x;
        vec3 sumC = vec3(0.0, 0.0, 0.0);
        float sumA = 0.0;
        float wsum = 0.0;
        for (int x = -2; x <= 2; ++x)
            for (int y = -2; y <= 2; ++y) {
                vec2 o = vec2(float(x), float(y)) * texel * offsScale;
                vec4 r = texture2DLod(s_hdr, uvTex + o, 0.0);
                float d = texture2DLod(s_depth, uvTex + o, 0.0).x;
                float w = exp(-abs(d - centerDepth) * 3000.0);
                sumC += r.rgb * w;
                sumA += r.a * w;
                wsum += w;
            }
        vec3 refl = sumC / max(wsum, 1e-4);
        float conf = clamp(sumA / max(wsum, 1e-4), 0.0, 1.0);
        vec3 scene = texture2DLod(s_scene, uvTex, 0.0).rgb;
        gl_FragColor = vec4(mix(scene, refl, conf), 1.0);
        return;
    }

    vec4 src = texture2D(s_hdr, uvTex);
    float reflectivity = step(0.5, src.a);

    vec3 P = ssrPosAt(uvTex, flip);

    vec4 gb = texture2DLod(s_gbuffer, uvTex, 0.0);
    vec3 N = normalize(gb.xyz * 2.0 - 1.0);
    vec3 V = normalize(P);
    if (dot(N, V) > 0.0)
        N = -N;
    vec3 R = normalize(reflect(V, N));

    float steps = max(u_ssrParams.z, 1.0);
    float stepLen = u_ssrParams.x / steps;
    float noise = fract(sin(dot(uvTex, vec2(12.9898, 78.233))) * 43758.5453);
    vec3 rayStep = R * stepLen;

    vec3 rayPos = P + rayStep * (0.5 + noise * 0.5);
    vec3 prevPos = P;
    vec3 hitColor = vec3(0.0, 0.0, 0.0);
    float hit = 0.0;
    float maxOvershoot = max(u_ssrParams.y, stepLen * 2.0);

    for (int i = 0; i < SSR_MAX_STEPS; ++i)
    {
        if (float(i) >= steps)
            break;

        vec4 clip = mul(u_ssrProj, vec4(rayPos, 1.0));
        if (clip.w > 0.0) {
            vec2 sRaw = (clip.xy / clip.w) * 0.5 + 0.5;
            vec2 sTex = sRaw;
            if (flip > 0.5)
                sTex.y = 1.0 - sTex.y;
            if (sRaw.x > 0.0 && sRaw.x < 1.0 && sRaw.y > 0.0 && sRaw.y < 1.0) {
                float sd = texture2DLod(s_depth, sTex, 0.0).x;
                vec3 scenePos = ssrViewPos(sRaw, sd);
                float delta = scenePos.z - rayPos.z;

                if (sd < 0.9999 && delta > 0.0 && delta < maxOvershoot) {
                    vec3 a = prevPos;
                    vec3 b = rayPos;
                    vec2 hUV = sTex;
                    vec2 hRaw = sRaw;
                    for (int j = 0; j < 6; ++j) {
                        vec3 m = (a + b) * 0.5;
                        vec4 mc = mul(u_ssrProj, vec4(m, 1.0));
                        vec2 mRaw = (mc.xy / mc.w) * 0.5 + 0.5;
                        vec2 mTex = mRaw;
                        if (flip > 0.5)
                            mTex.y = 1.0 - mTex.y;
                        float md = texture2DLod(s_depth, mTex, 0.0).x;
                        vec3 mScene = ssrViewPos(mRaw, md);
                        if (mScene.z - m.z > 0.0) {
                            b = m;
                            hUV = mTex;
                            hRaw = mRaw;
                        } else {
                            a = m;
                        }
                    }

                    vec2 e = smoothstep(0.0, 0.12, hRaw) * smoothstep(0.0, 0.12, 1.0 - hRaw);
                    float travel = length(b - P);
                    float distFade = 1.0 - smoothstep(u_ssrParams.x * 0.6, u_ssrParams.x, travel);
                    hit = e.x * e.y * distFade;
                    hitColor = texture2DLod(s_hdr, hUV, 0.0).rgb;
                    break;
                }
            }
        }
        prevPos = rayPos;
        rayPos += rayStep;
    }

    gl_FragColor = vec4(hitColor, reflectivity * hit * u_ssrParams.w);
}
