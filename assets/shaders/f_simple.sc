$input v_color0, v_normal, v_position, v_texcoord0

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "shaderlib.sh"
#include "material.sc"

uniform vec4 u_color;

void main() {
    vec4 finalColor = u_color * v_color0;

    finalColor = mix(finalColor, u_baseColor * v_color0, step(0.001, u_baseColor.a));

    finalColor.xyz += u_emissive.xyz * 0.1;

    if (finalColor.a < 0.1) {
        finalColor = vec4(1.0, 0.0, 0.0, 1.0);
    }

    gl_FragColor = finalColor;
}