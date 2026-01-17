$input v_position, v_normal, v_tangent, v_texcoord0, v_viewDir

#include <bgfx_shader.sh>
#include "material.sc"

void main()
{
    Material mat;
    mat.baseColor = u_baseColor;
    mat.metallic = u_materialParams.x;
    mat.roughness = u_materialParams.y;
    mat.normal = normalize(v_normal);
    mat.emissive = u_emissive.xyz;

    mat = initMaterial(mat);

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    vec3 viewDir = normalize(v_viewDir);

    vec3 radiance = vec3(1.0, 1.0, 1.0); // Light color and intensity
    vec3 Lo = cookTorrance(mat.normal, viewDir, lightDir, mat);

    vec3 ambient = vec3(0.03, 0.03, 0.03) * mat.baseColor.rgb;

    vec3 color = ambient + Lo * radiance + mat.emissive;

    color = color / (color + vec3(1.0, 1.0, 1.0));
    color = pow(max(color, vec3(0.0, 0.0, 0.0)), vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    gl_FragColor = vec4(color, mat.baseColor.a);
}
