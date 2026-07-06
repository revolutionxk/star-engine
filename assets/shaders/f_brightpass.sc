$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_hdr, 0);

uniform vec4 u_bloomParams;

void main()
{
    vec3 c = texture2D(s_hdr, v_texcoord0).rgb;
    float luma = dot(c, vec3(0.2126, 0.7152, 0.0722));

    float threshold = u_bloomParams.x;
    float knee = max(u_bloomParams.y, 0.0001);
    float soft = clamp((luma - threshold + knee) / (2.0 * knee), 0.0, 1.0);
    float contribution = max(soft * soft, step(threshold, luma)) * max(luma - threshold, 0.0) / max(luma, 0.0001);

    gl_FragColor = vec4(c * contribution, 1.0);
}
